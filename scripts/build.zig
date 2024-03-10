const std = @import("std");

pub fn build(b: *std.Build) void {
    const runner = b.addExecutable(.{
        .name = "run_with_tb",
        .target = b.standardTargetOptions(.{}),
        .optimize = b.standardOptimizeOption(.{}),
        .root_source_file = .{ .path = "run_with_tb.zig" },
    });
    add_zig_files(b, runner, &.{
        "tigerbeetle.zig",
        "constants.zig",
        "storage.zig",
        "io.zig",
        "message_pool.zig",
        "message_bus.zig",
        "state_machine.zig",
        "ring_buffer.zig",
        "vsr.zig",
        "shell.zig",
        "stdx.zig",
        "clients/shutil.zig",
    });
    runner.linkLibC();

    // Run executable file
    const run_cmd = b.addRunArtifact(runner);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", b.fmt("Run the {s} app", .{runner.name}));
    run_step.dependOn(&run_cmd.step);
}

fn add_zig_files(b: *std.Build, exe: *std.Build.Step.Compile, comptime files: []const []const u8) void {
    const options = b.addOptions();
    const ConfigBase = enum {
        production,
        development,
        test_min,
        default,
    };

    options.addOption(
        ConfigBase,
        "config_base",
        .default,
    );

    options.addOption(
        std.log.Level,
        "config_log_level",
        .info,
    );

    const TracerBackend = enum {
        none,
        perfetto,
        tracy,
    };
    options.addOption(TracerBackend, "tracer_backend", .none);

    const aof_record_enable = b.option(bool, "config-aof-record", "Enable AOF Recording.") orelse false;
    const aof_recovery_enable = b.option(bool, "config-aof-recovery", "Enable AOF Recovery mode.") orelse false;
    options.addOption(bool, "config_aof_record", aof_record_enable);
    options.addOption(bool, "config_aof_recovery", aof_recovery_enable);

    const HashLogMode = enum {
        none,
        create,
        check,
    };
    options.addOption(HashLogMode, "hash_log_mode", .none);
    const shutil = options.createModule();

    inline for (files) |file| {
        var pkg = b.createModule(.{
            .source_file = .{ .path = "../build/_deps/tb-src/src/" ++ file },
            .dependencies = &.{
                .{
                    .name = "shutil",
                    .module = shutil,
                },
            },
        });
        exe.addModule(file, pkg);
    }
}
