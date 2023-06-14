# TigerBeetle C++ client (Header only)

[TigerBeetle] is a financial accounting database designed for mission critical safety and performance to power the future of financial services.

### Prerequisites

 C++ version: 17

**Libraries**
- fmtlib v10.0.0
- TigerBeetle C client library v0.13.36

**Tools**
- cmake v3.14 or higher
- zig v0.9.1 (tigerbeetle compatible)


### How to Build and Run

**Another C++ toolchain**

```bash
$> cmake -B build -DTIGERBEETLE_BUILD_SHARED_LIBS=ON # (tb_client.[so|dll|dylib])
$> cmake --build build --target run_with_tb # run TigerBeetle server + C++ client 
```

**Zig toolchain**

```bash
# Linux/MacOS
$> cmake -B build -DCMAKE_CXX_COMPILER=scripts/zigcxx.sh
# Windows
$> cmake -B build -DCMAKE_CXX_COMPILER=scripts/zigcxx.cmd
# both
$> cmake --build build --target run_with_tb # run TigerBeetle server + C++ client 
```

### Build tests

See sample: [example/basic.cpp](example/basic.cpp)

<details>
<summary>Output</summary>

```bash
# possible output
[100%] Built target tb_cpp
Running tb_cpp with TigerBeetle
Starting replica 0

running client...
error(message_bus): error connecting to replica 0: error.ConnectionRefused
info(message_bus): connected to replica 0
TigerBeetle C++ Sample

[trace] Connecting...
[trace] Creating accounts...
[info] Accounts created successfully
[trace] Creating transfers...
[info] Transfers created successfully
============================================
[trace] 194 transfers per second
[trace] create_transfers max p100 latency per 8191 transfers = 1294686ms
[trace] total 819100 transfers in 4200636ms
[info] Looking up accounts ...
[info] 2 Account(s) found
============================================
[trace] id=1
[trace] debits_posted=819100
[trace] credits_posted=0
[trace] id=2
[trace] debits_posted=0
[trace] credits_posted=819100

Done!!
Killing tigerbeetle start process...
Terminating tigerbeetle start process...
[100%] Built target run_with_tb
```
</details>

| Compiler | Tested | `tb_client` library |
| --- | --- | --- |
| GCC | 🆗 | Shared |
| GCC | ❌ | Static |
| Clang | 🆗 | Shared |
| Clang | ❌ | Static |
| AppleClang | 🆗 | Shared |
| AppleClang | ❌ | Static |
| zig `cc/c++` | 🆗 | Shared |
| zig `cc/c++` | 🆗 | Static |
| MSVC | None | Shared |
| MSVC | None | Static |

**Note:** `zig c++` equal to `clang++ -stdlib=libc++ -fuse-ld=lld` for all targets (builtin), except to MacOS target, replacing `lld` to [`zld`](https://github.com/kubkon/zld)!!

## TODO

- [ ] upgrade zig 0.9.1 (stage1) to zig 0.11.0 (stage3) - pkg manager (choose cmake or zig only)


## Frequently Asked Questions

#### Do I need to install Zig to compile this project?

**A:** No! You can use the C and/or C++ compiler of your choice.

However, as mentioned in issue [#3](https://github.com/kassane/tigerbeetle-cpp/issues/3), it will only be possible to link dynamically. The Zig static library does not include `compiler-rt` library, it only includes executables and shared libraries.

- [undefined reference to `__zig_probe_stack`](https://github.com/tigerbeetledb/tigerbeetle/pull/792)

It is also not limited to C++, you just need to modify the `CMakeLists.txt` to use it with other languages supported by CMake.

#### Is this project an official binding for TigerBeetleDB?

**A:** No! It's only a community project. But there's nothing stopping the main developers from porting it to the official repository if they want to.

#### What is the goal of `tigerbeetle-cpp`?

**A:** Firstly, to provide a simple C++ solution derived from the [C binding][client-c] (thanks to [@batiati](https://github.com/batiati)).

Also, to demonstrate that it's possible to use Zig with CMake to build the `tb_client` library without the user needing to use the Zig toolchain (C and/or C++). They don't even need to learn it if they don't want to. (I suggest reconsidering this matter!!)

Another important point is that TigerBeetle is a product in the early stages of development, subject to flaws and mistakes. The best way to improve this new tool is by testing, and that's where the relevance of the clients (language bindings) comes into play.

## References


| language | binding | description |
| --- | --- | --- |
| **C** | [client-c] | Official |
| **Elixir** | [tigerbeetlex] | Community |
| **Go** | [client-go] | Official |
| **Java** | [client-java] | Official |
| **.Net** | [client-dotnet] | Official |
| **Node** | [client-node] | Official |
| **Rust** | [tigerbeetle-rs] | Community |

## License

See: [LICENSE](LICENSE)

[TigerBeetle]: https://github.com/tigerbeetledb/tigerbeetle
[client-c]:https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/c
[client-go]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/go
[client-node]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/node
[client-java]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/java
[client-dotnet]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/dotnet
[tigerbeetlex]: https://github.com/rbino/tigerbeetlex
[tigerbeetle-rs]: https://github.com/ZetaNumbers/tigerbeetle-rs
