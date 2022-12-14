const std = @import("std");
const common = @import("common");
const FileLineReader = common.FileLineReader;
const stdout = std.io.getStdOut().writer();

const use_test_input = false;

const input_filename: [:0]const u8 = if (use_test_input) "test_input" else "input";
const output_ppm = true;

const grid_size = 1024;

const Point = struct {
    x: i32 = 0,
    y: i32 = 0,

    pub fn init(x: i32, y: i32) Point {
        return .{ .x = x, .y = y };
    }
};

const Color = struct {
    r: u8 = 0,
    g: u8 = 0,
    b: u8 = 0,

    const white = Color{ .r = 255, .g = 255, .b = 255 };
    const black = Color{ .r = 0, .g = 0, .b = 0 };
    const red = Color{ .r = 255, .g = 0, .b = 0 };
    const green = Color{ .r = 0, .g = 255, .b = 0 };
    const blue = Color{ .r = 0, .g = 0, .b = 255 };

    const sand = Color{ .r = 236, .g = 198, .b = 145 };
    const cave = Color{ .r = 76, .g = 92, .b = 107 };
    const rock = Color{ .r = 80, .g = 154, .b = 173 };
    const emitter = Color{ .r = 215, .g = 91, .b = 91 };
};

const Scan = struct {
    grid: [grid_size][grid_size]bool = [_][grid_size]bool{[_]bool{false} ** grid_size} ** grid_size,
    sand_grid: [grid_size][grid_size]bool = [_][grid_size]bool{[_]bool{false} ** grid_size} ** grid_size,

    min_x: usize = std.math.maxInt(usize),
    max_x: usize = 500,
    min_y: usize = 0,
    max_y: usize = 0,
    pour_point: Point = .{ .x = 500, .y = 0 },
    sand_point: Point = Point.init(0, 0),
    num_deposited: usize = 0,

    pub fn mark(self: *Scan, p: Point) void {
        if (p.x < self.min_x) self.min_x = @intCast(usize, p.x);
        if (p.x > self.max_x) self.max_x = @intCast(usize, p.x);
        if (p.y < self.min_y) self.min_y = @intCast(usize, p.y);
        if (p.y > self.max_y) self.max_y = @intCast(usize, p.y);

        self.grid[@intCast(usize, p.x)][@intCast(usize, p.y)] = true;
    }

    pub fn markPath(self: *Scan, p: []const Point) void {
        var i: usize = 0;
        while (i < (p.len - 1)) : (i += 1) {
            var p0: Point = p[i];
            var p1: Point = p[i + 1];

            if (p0.x == p1.x) {
                var min_y = if (p0.y < p1.y) p0.y else p1.y;
                var max_y = if (p0.y < p1.y) p1.y else p0.y;
                var y = min_y;

                while (y <= max_y) : (y += 1) {
                    self.mark(.{ .x = p0.x, .y = y });
                }
            } else {
                var min_x = if (p0.x < p1.x) p0.x else p1.x;
                var max_x = if (p0.x < p1.x) p1.x else p0.x;
                var x = min_x;

                while (x <= max_x) : (x += 1) {
                    self.mark(.{ .x = x, .y = p0.y });
                }
            }
        }
    }

    pub fn printScan(self: *Scan) !void {
        var x: usize = self.min_x;
        var y: usize = self.min_y;

        try stdout.print("bounds: ({}, {}), ({}, {})\n", .{ self.min_x, self.max_x, self.min_y, self.max_y });

        while (y <= self.max_y) : (y += 1) {
            while (x <= self.max_x) : (x += 1) {
                var ch: u8 = if (self.grid[x][y]) '#' else '.';

                if (x == self.pour_point.x and y == self.pour_point.y) {
                    ch = '+';
                }

                try stdout.print("{c}", .{ch});
            }
            x = self.min_x;
            try stdout.print("\n", .{});
        }
    }

    pub fn ppm(self: *Scan, filename: []const u8) !void {
        const f = try std.fs.cwd().createFile(filename, .{});
        defer f.close();
        // var w = f.writer();
        var bw = std.io.bufferedWriter(f.writer());

        var w = bw.writer();

        var width = self.max_x - self.min_x + 1;
        var height = self.max_y - self.min_y + 1;
        try w.print("P3\n{} {}\n255\n", .{ width, height });

        var x: usize = self.min_x;
        var y: usize = self.min_y;

        while (y <= self.max_y) : (y += 1) {
            while (x <= self.max_x) : (x += 1) {
                var kind: u8 = if (self.grid[x][y]) 1 else 0;

                if (self.sand_grid[x][y] or (x == self.sand_point.x and y == self.sand_point.y)) {
                    kind = 3;
                }

                if (x == self.pour_point.x and y == self.pour_point.y) {
                    kind = 2;
                }

                var c = switch (kind) {
                    0 => Color.cave,
                    1 => Color.rock,
                    2 => Color.emitter,
                    3 => Color.sand,
                    else => Color.blue,
                };

                try w.print("{} {} {} ", .{ c.r, c.g, c.b });
            }
            x = self.min_x;
            try w.print("\n", .{});
        }

        try bw.flush();
    }

    fn can_move_down(self: *Scan) bool {
        var p = &self.sand_point;
        return !self.grid[@intCast(usize, p.x)][@intCast(usize, p.y + 1)] and !self.sand_grid[@intCast(usize, p.x)][@intCast(usize, p.y + 1)];
    }

    fn can_move_sw(self: *Scan) bool {
        var p = &self.sand_point;
        return !self.grid[@intCast(usize, p.x - 1)][@intCast(usize, p.y + 1)] and !self.sand_grid[@intCast(usize, p.x - 1)][@intCast(usize, p.y + 1)];
    }

    fn can_move_se(self: *Scan) bool {
        var p = &self.sand_point;
        return !self.grid[@intCast(usize, p.x + 1)][@intCast(usize, p.y + 1)] and !self.sand_grid[@intCast(usize, p.x + 1)][@intCast(usize, p.y + 1)];
    }

    fn deposit_sand(self: *Scan) void {
        var p = &self.sand_point;
        self.sand_grid[@intCast(usize, p.x)][@intCast(usize, p.y)] = true;
        self.num_deposited += 1;
    }

    pub fn simulate(self: *Scan) !void {
        self.sand_point.x = 500;
        self.sand_point.y = 0;

        var frame: usize = 1;
        var buf: [128]u8 = undefined;

        while (true) {
            if (output_ppm) {
                try self.ppm(try std.fmt.bufPrint(&buf, "ppm/img{}.ppm", .{frame}));
                try stdout.print("frame: {}\n", .{frame});
                frame += 1;
            }

            if (self.can_move_down()) {
                self.sand_point.y += 1;
            } else if (self.can_move_sw()) {
                self.sand_point.y += 1;
                self.sand_point.x -= 1;
            } else if (self.can_move_se()) {
                self.sand_point.y += 1;
                self.sand_point.x += 1;
            } else {
                self.deposit_sand();
                self.sand_point.x = 500;
                self.sand_point.y = 0;
                continue;
            }

            if (self.sand_point.y > self.max_y) {
                break;
            }
        }

        if (output_ppm) {
            try self.ppm(try std.fmt.bufPrint(&buf, "ppm/out_{:3}.ppm", .{frame}));
        }
    }

    fn simulate_part_2(self: *Scan) !void {
        self.sand_point.x = 500;
        self.sand_point.y = 0;

        while (true) {
            if (self.sand_grid[500][0]) {
                break;
            }

            if (self.can_move_down()) {
                self.sand_point.y += 1;
            } else if (self.can_move_sw()) {
                self.sand_point.y += 1;
                self.sand_point.x -= 1;
            } else if (self.can_move_se()) {
                self.sand_point.y += 1;
                self.sand_point.x += 1;
            } else {
                self.deposit_sand();
                self.sand_point.x = 500;
                self.sand_point.y = 0;
                continue;
            }

            if (self.sand_point.y >= self.max_y + 1) {
                self.deposit_sand();
                self.sand_point.x = 500;
                self.sand_point.y = 0;
                continue;
            }

            //if (self.sand_point.y > self.max_y) {
            //    break;
            //}
        }
    }
};

const State = enum {
    ParsingPair,
    WaitingForPair,
};

const Path = [128]Point;

pub fn main() !void {
    const f = try std.fs.cwd().openFile(input_filename, .{});
    var r = FileLineReader(1024).init(f);

    var state: State = .WaitingForPair;
    var scan: Scan = .{};

    while (try r.nextLine()) |line| {
        var cur: usize = 0;
        var path: Path = [_]Point{.{}} ** 128;
        var point_count: usize = 0;
        var current_point = &path[0];
        while (cur < line.len) {
            var ch = line[cur];
            cur += 1;

            switch (ch) {
                '0'...'9' => {
                    var start = cur - 1;
                    ch = line[cur];
                    while (std.ascii.isDigit(ch)) {
                        cur += 1;
                        ch = line[cur];
                    }

                    var num = try std.fmt.parseInt(i32, line[start..cur], 10);

                    switch (state) {
                        .WaitingForPair => {
                            current_point.x = num;
                            state = .ParsingPair;
                        },
                        .ParsingPair => {
                            current_point.y = num;
                            state = .WaitingForPair;
                            point_count += 1;
                            current_point = &path[point_count];
                        },
                    }
                },
                ',' => {},
                ' ' => {},
                '-' => {},
                '\n' => {},
                else => {},
            }
        }

        scan.markPath(path[0..point_count]);
    }

    try scan.printScan();

    try scan.simulate();
    try stdout.print("num deposited: {}\n", .{scan.num_deposited});

    try scan.simulate_part_2();
    try stdout.print("num deposited (part 2): {}\n", .{scan.num_deposited});
}
