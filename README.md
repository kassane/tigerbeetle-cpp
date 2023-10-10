# TigerBeetle C++ client (Header only)

[TigerBeetle] is a financial accounting database designed for mission critical safety and performance to power the future of financial services.

### Prerequisites

 C++ version: 17

**Libraries**
- fmtlib v10.1.1
- TigerBeetle C client library (latest version - branch main)

**Tools**
- cmake v3.14 or higher
- zig v0.11.0 (tigerbeetle compatible)


### How to Build and Run

**Another C++ toolchain**

```bash
$> cmake -B build # (tb_client.[a|lib])
# OR
$> cmake -B build -DTIGERBEETLE_BUILD_SHARED_LIBS=ON # (tb_client.[so|dll|dylib])
# Build and test - build client examples
$> cmake --build build -DBUILD_EXAMPLES=ON --target run_with_tb # run TigerBeetle server + your client 
```

**Zig toolchain**

```bash
# Linux/MacOS
$> cmake -B build -DCMAKE_CXX_COMPILER=scripts/zigcxx.sh
# Windows
$> cmake -B build -DCMAKE_CXX_COMPILER=scripts/zigcxx.cmd
```

### How to use

- Add on your cmake project:

```cmake
include(FetchContent)

find_package(TigerBeetle 0.3.0 QUIET)
if (NOT TigerBeetle_FOUND)
    FetchContent_Declare(TigerBeetle GIT_REPOSITORY https://github.com/kassane/tigerbeetle-cpp.git
        GIT_TAG main)
    FetchContent_GetProperties(TigerBeetle)
# required
    set(APP_TARGETS ${PROJECT_NAME}) # executable or executables (need foreach) names
# optional
#   set(TB_VERSION 0.13.137) # tigerbeetle branch/tag repo (default: main)
#   set(TB_ADDRESS 3000) # tb_server port (default: 3001)
    FetchContent_MakeAvailable(TigerBeetle)
endif()

# linking your app to tb_client library
target_link_libraries(${PROJECT_NAME}
    PRIVATE tb_client
)
target_include_directories(${PROJECT_NAME} PUBLIC ${TigerBeetle_SOURCE_DIR}/include)
target_link_directories(${PROJECT_NAME} PUBLIC ${TigerBeetle_BINARY_DIR})
```

### Build Samples

**See:**
- [examples/basic.cpp](examples/basic.cpp)
- [examples/two_phase.cpp](examples/two_phase.cpp)
- [examples/two_phase_many.cpp](examples/two_phase_many.cpp)

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
| GCC | 🆗 | Static |
| Clang | 🆗 | Shared |
| Clang | 🆗 | Static |
| AppleClang | 🆗 | Shared |
| AppleClang | 🆗 | Static |
| zig `cc/c++` | 🆗 | Shared |
| zig `cc/c++` | 🆗 | Static |
| MSVC | None | Shared |
| MSVC | None | Static |

**Note:** `zig c++` equal to `clang++ -stdlib=libc++ -fuse-ld=lld` for all targets (builtin), except to MacOS target, replacing `lld` to [`zld`](https://github.com/kubkon/zld)!!

## TODO

- [ ] zig 0.11.0 - missing zig-pkg (choose cmake or zig only) to `c_client` support.


## Frequently Asked Questions

#### Do I need to install Zig to compile this project?

**A:** No! You can use the C and/or C++ compiler of your choice.

However, as mentioned in issue [#3](https://github.com/kassane/tigerbeetle-cpp/issues/3), it will only be possible to link dynamically. The Zig static library does not include `compiler-rt` library, it only includes executables and shared libraries.

- [undefined reference to `__zig_probe_stack`](https://github.com/tigerbeetledb/tigerbeetle/pull/792) - **Fixed**.

It is also not limited to C++, you just need to modify the `CMakeLists.txt` to use it with other languages supported by CMake.

> :information_source: CMake: Supported languages are C, CXX (i.e. C++), CSharp (i.e. C#), CUDA, OBJC (i.e. Objective-C), OBJCXX (i.e. Objective-C++), Fortran, HIP, ISPC, Swift, ASM, ASM_NASM, ASM_MARMASM, ASM_MASM, and ASM-ATT.

**See:** [cmake docs](https://cmake.org/cmake/help/latest/command/enable_language.html)


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
