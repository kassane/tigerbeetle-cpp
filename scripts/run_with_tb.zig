//! This script spins up a TigerBeetle server on an available
//! TCP port and references a brand new data file. Then it runs the
//! command passed to it and shuts down the server. It cleans up all
//! resources unless told not to. It fails if the command passed to it
//! fails. It could have been a bash script except for that it works on
//! Windows as well.
//!
//! Example: (run from the repo root)
//!   ./zig/zig build run_with_tb -- node myscript.js
//!

const std = @import("std");
const builtin = @import("builtin");
const log = std.log.scoped(.run_with_tb);
const os = std.os;

const run = @import("clients/shutil.zig").run;
const run_with_env = @import("clients/shutil.zig").run_with_env;
const TmpDir = @import("clients/shutil.zig").TmpDir;
const TB_root = @import("clients/shutil.zig").TB_root;
const path_exists = @import("clients/shutil.zig").path_exists;
const script_filename = @import("clients/shutil.zig").script_filename;
const binary_filename = @import("clients/shutil.zig").binary_filename;
const file_or_directory_exists = @import("clients/shutil.zig").file_or_directory_exists;

pub fn run_with_tb(
    arena: *std.heap.ArenaAllocator,
    commands: []const []const u8,
    cwd: []const u8,
) !void {
    try run_many_with_tb(arena, &[_][]const []const u8{commands}, cwd);
}

pub fn run_many_with_tb(
    arena: *std.heap.ArenaAllocator,
    commands: []const []const []const u8,
    cwd: []const u8,
) !void {
    const allocator = arena.allocator();
    var tmp_dir = std.testing.tmpDir(.{});
    errdefer tmp_dir.cleanup();
    const tmp_dir_path = try tmp_dir.dir.realpathAlloc(allocator, ".");
    defer allocator.free(tmp_dir_path);
    const data_file: []const u8 = try std.fs.path.join(allocator, &.{ tmp_dir_path, "0_0.tigerbeetle" });
    defer allocator.free(data_file);

    var map = std.process.EnvMap.init(allocator);
    defer map.deinit();

    const port = map.get("TB_ADDRESS");
    const tb_addr = try std.fmt.allocPrint(allocator, "--addresses={s}", .{if (port) |p| p else "3001"});

    var tb_fmt = try std.ChildProcess.run(.{
        .allocator = allocator,
        .argv = &.{ "../build/_deps/tb-src/zig-out/bin/tigerbeetle", "format", "--cluster=0", "--replica=0", "--replica-count=1", data_file },
        .cwd = cwd,
    });
    _ = tb_fmt;

    log.info("Running commands: {s}", .{commands});

    try std.os.chdir(cwd);
    var tb_server = std.ChildProcess.init(&.{ "../build/_deps/tb-src/zig-out/bin/tigerbeetle", "start", tb_addr, data_file }, allocator);
    tb_server.cwd = cwd;

    try tb_server.spawn();
    errdefer {
        _ = tb_server.kill() catch unreachable;
    }
    for (commands) |command_argv| {
        try run_with_env(
            arena,
            command_argv,
            if (port) |v| &.{ "TB_ADDRESS", v } else &.{},
        );
    }
}

fn error_main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const cwd = std.process.getEnvVarOwned(allocator, "R_CWD") catch ".";

    var collected_args = std.ArrayList([]const u8).init(allocator);
    var args = try std.process.argsWithAllocator(allocator);
    defer args.deinit();

    // Skip first arg, this process's name
    std.debug.assert(args.skip());
    while (args.next()) |arg| {
        try collected_args.append(arg);
    }

    try run_with_tb(&arena, collected_args.items, cwd);
}

// Returning errors in main produces useless traces, at least for some
// known errors. But using errors allows defers to run. So wrap the
// main but don't pass the error back to main. Just exit(1) on
// failure.
pub fn main() !void {
    if (error_main()) {
        // fine
    } else |err| switch (err) {
        error.RunCommandFailed => std.os.exit(1),
        else => return err,
    }
}
