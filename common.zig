const std = @import("std");

pub fn FileLineReader(comptime max_line_len: usize) type {
    return struct {
        buf: [max_line_len]u8 = undefined,
        cur: usize = 0,
        f: std.fs.File,

        const Self = @This();

        pub fn init(f: std.fs.File) Self {
            return .{ .f = f };
        }

        fn pushChar(self: *Self, ch: u8) !void {
            if (self.cur >= max_line_len) {
                return error.BufferOverflow;
            }

            defer self.cur += 1;
            self.buf[self.cur] = ch;
        }

        pub fn nextLine(self: *Self) !?[]u8 {
            var r = self.f.reader();
            self.cur = 0;
            while (true) {
                var ch = r.readByte() catch {
                    return null;
                };

                try self.pushChar(ch);

                if (ch == '\n') {
                    break;
                }
            }

            return self.buf[0..self.cur];
        }
    };
}
