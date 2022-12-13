const std = @import("std");
const common = @import("common");
const FileLineReader = common.FileLineReader;

const use_test_input = false;

const input_filename: [:0]const u8 = if (use_test_input) "test_input" else "input";
const num_packets = if (use_test_input) (16 + 2) else (300 + 2);

const max_nodes = 128;
const max_children = 32;

const NodeType = enum {
    Number,
    List,
    None,
};

const Node = struct {
    node_type: NodeType = .None,
    value: i32 = -1,
    children: [max_children]?*Node = [_]?*Node{null} ** max_children,
    children_count: usize = 0,

    pub fn addChild(self: *Node, child: *Node) !void {
        if (self.children_count >= max_children) {
            return error.MaxChildrenReachedForNode;
        }

        defer self.children_count += 1;
        self.children[self.children_count] = child;
    }

    pub fn print(self: *Node, w: anytype) !void {
        switch (self.node_type) {
            .Number => {
                try w.print("(Number: {})", .{self.value});
            },
            .List => {
                try w.print("(List: ", .{});
                var i: usize = 0;
                while (i < self.children_count) : (i += 1) {
                    try self.children[i].?.print(w);
                }
                try w.print(")", .{});
            },
            .None => {
                try w.print("(None)", .{});
            },
        }
    }
};

const Packet = struct {
    root: Node = Node{},
    nodes: [max_nodes]Node = [_]Node{.{}} ** max_nodes,
    node_count: usize = 0,

    pub fn allocNode(self: *Packet) !*Node {
        if (self.node_count >= max_nodes) {
            return error.OutOfNodes;
        }

        defer self.node_count += 1;
        return &self.nodes[self.node_count];
    }

    pub fn allocNumberNode(self: *Packet, value: i32) !*Node {
        var result = try self.allocNode();
        result.value = value;
        result.children_count = 0;
        result.node_type = .Number;
        return result;
    }

    pub fn allocListNode(self: *Packet) !*Node {
        var result = try self.allocNode();
        result.node_type = .List;
        return result;
    }

    pub fn print(self: *Packet, w: anytype) !void {
        try w.print("Packet: ", .{});
        try self.root.print(w);
        try w.print("\n", .{});
    }
};

const Parser = struct {
    packet: *Packet,
    buf: []const u8,
    cur: usize = 0,

    fn next(self: *Parser) ?u8 {
        if (self.cur >= self.buf.len) {
            return null;
        }

        defer self.cur += 1;
        return self.buf[self.cur];
    }

    fn peek(self: *Parser) ?u8 {
        if (self.cur >= self.buf.len) {
            return null;
        }

        return self.buf[self.cur];
    }

    fn skip(self: *Parser) void {
        self.cur += 1;
    }

    fn skipIfChar(self: *Parser, needle: u8) void {
        if (self.peek()) |ch| {
            if (ch == needle) {
                self.skip();
            }
        }
    }

    pub fn init(packet: *Packet, line: []const u8) Parser {
        return .{
            .packet = packet,
            .buf = line,
            .cur = 0,
        };
    }

    fn parseNumber(self: *Parser) !i32 {
        var start = self.cur;
        var end = start;

        while (self.peek()) |ch| {
            if (std.ascii.isDigit(ch)) {
                self.skip();
            } else {
                end = self.cur;
                break;
            }
        } else {
            return error.ParseError;
        }

        return try std.fmt.parseInt(i32, self.buf[start..end], 10);
    }

    fn parseListNode(self: *Parser, node: *Node) !void {
        node.node_type = .List;

        while (self.peek()) |ch| {
            switch (ch) {
                '[' => {
                    self.skip();
                    var child = try self.packet.allocListNode();
                    try node.addChild(child);
                    try self.parseListNode(child);
                },
                ']' => {
                    self.skip();
                    return;
                },
                ',' => {
                    self.skip();
                },
                else => {
                    if (std.ascii.isDigit(ch)) {
                        var child = try self.packet.allocNode();
                        try self.parseNumberNode(child);
                        try node.addChild(child);
                    } else {
                        return error.ListNodeParseError;
                    }
                },
            }
        }
    }

    fn parseNumberNode(self: *Parser, node: *Node) !void {
        node.node_type = .Number;
        node.value = try self.parseNumber();
    }

    fn parseNode(self: *Parser, node: *Node) !void {
        if (self.peek()) |ch| {
            if (ch == '[') {
                self.skip();
                try self.parseListNode(node);
            } else if (std.ascii.isDigit(ch)) {
                try self.parseNumberNode(node);
            } else {
                return error.ParseError;
            }
        }
    }

    pub fn parse(self: *Parser) !void {
        try self.parseNode(&self.packet.root);
    }
};

const CompareResult = enum {
    RightOrder,
    Equal,
    WrongOrder,
};

fn compareNodes(lhs: *Node, rhs: *Node) CompareResult {
    if (lhs.node_type == .List and rhs.node_type == .List) {
        var i: usize = 0;
        while (i < lhs.children_count and i < rhs.children_count) : (i += 1) {
            var result = compareNodes(lhs.children[i].?, rhs.children[i].?);

            if (result != .Equal) {
                return result;
            }
        } else {
            if (lhs.children_count == rhs.children_count) {
                return .Equal;
            } else if (lhs.children_count < rhs.children_count) {
                return .RightOrder;
            } else {
                return .WrongOrder;
            }
        }
    } else if (lhs.node_type == .Number and rhs.node_type == .Number) {
        if (lhs.value == rhs.value) {
            return .Equal;
        } else if (lhs.value < rhs.value) {
            return .RightOrder;
        } else {
            return .WrongOrder;
        }
    } else {
        if (lhs.node_type == .Number) {
            var node = Node{};
            node.node_type = .List;
            node.children_count = 1;
            node.children[0] = lhs;

            return compareNodes(&node, rhs);
        } else {
            var node = Node{};
            node.node_type = .List;
            node.children_count = 1;
            node.children[0] = rhs;

            return compareNodes(lhs, &node);
        }
    }
}

fn addDividerPacket(packet_list: []Packet, packet_count: *usize, value: i32) !void {
    var divider_packet_1 = &packet_list[packet_count.*];
    packet_count.* += 1;
    var n1 = try divider_packet_1.allocNode();
    var n2 = try divider_packet_1.allocNode();
    n1.node_type = .List;
    n2.node_type = .Number;
    n2.value = value;
    try n1.addChild(n2);
    divider_packet_1.root.node_type = .List;
    try divider_packet_1.root.addChild(n1);
}

pub fn main() !void {
    var stdout = std.io.getStdOut().writer();
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    var allocator = arena.allocator();

    var f = try std.fs.cwd().openFile(input_filename, .{});
    defer f.close();

    var line_reader = FileLineReader(1024).init(f);

    var count: usize = 0;
    var pair_index: usize = 1;
    var sum: usize = 0;
    var packet_list = try allocator.alloc(Packet, num_packets);
    for (packet_list) |*p| {
        p.* = Packet{};
    }
    var packet_count: usize = 0;

    while (try line_reader.nextLine()) |line| {
        if (line.len == 1) {
            continue;
        }

        var parser = Parser.init(&packet_list[packet_count], line);
        try parser.parse();
        packet_count += 1;

        count += 1;

        if (count > 2) {
            @panic("Invalid input file format!");
        }

        if (count == 2) {
            var result = compareNodes(&packet_list[packet_count - 2].root, &packet_list[packet_count - 1].root);
            if (result != .WrongOrder) {
                sum += pair_index;
            } else {}

            count = 0;
            pair_index += 1;
        }
    }

    try stdout.print("Sum of pair indices which are correctly ordererd: {}\n", .{sum});

    try addDividerPacket(packet_list, &packet_count, 2);
    try addDividerPacket(packet_list, &packet_count, 6);

    if (packet_count != num_packets) {
        @panic("Invalid packet count!");
    }

    var indices = try allocator.alloc(usize, num_packets);
    var i: usize = 0;
    while (i < num_packets) : (i += 1) {
        indices[i] = i;
    }

    const Sorter = struct {
        const Context = struct {
            packet_list: []const Packet,
        };

        pub fn lessThanFn(ctx: Context, lhs: usize, rhs: usize) bool {
            var lhs_packet = ctx.packet_list[lhs];
            var rhs_packet = ctx.packet_list[rhs];

            var result = compareNodes(&lhs_packet.root, &rhs_packet.root);

            if (result == .RightOrder) {
                return true;
            } else {
                return false;
            }
        }
    };

    std.sort.sort(
        usize,
        indices,
        Sorter.Context{ .packet_list = packet_list },
        Sorter.lessThanFn,
    );

    var decoder_key: usize = 1;
    count = 0;
    for (indices) |index| {
        var packet = &packet_list[index];

        if (packet.root.children_count == 1) {
            var child = packet.root.children[0].?;
            if (child.children_count == 1) {
                var chilchild = child.children[0].?;
                var value = chilchild.value;
                if (value == 2 or value == 6) {
                    decoder_key *= (count + 1);
                }
            }
        }

        count += 1;
    }

    try stdout.print("decoder key: {}", .{decoder_key});
}
