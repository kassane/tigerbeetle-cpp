# TigerBeetle C++ client

[TigerBeetle] is a financial accounting database designed for mission critical safety and performance to power the future of financial services.

### Prerequisites

 C++ version: 14

**Libraries**
- fmtlib v10.0.0
- TigerBeetle C client library v0.13.12

**Tools**
- cmake v3.14 or higher
- zig v0.9.1 (tigerbeetle compatible)


## How to run

```bash
# Linux/MacOS
$> cmake -B build -DCMAKE_CXX_COMPILER=scripts/zigcxx.sh
# Windows
$> cmake -B build -DCMAKE_CXX_COMPILER=scripts/zigcxx.cmd
# both
$> cmake --build build --target run_with_tb # run TigerBeetle server + C++ client 
```
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


### TODO

- [ ] upgrade zig 0.9.1 (stage1) to zig 0.11.0 (stage3) - pkg manager (choose cmake or zig only)


## References


| language | binding | description |
| --- | --- | --- |
| **C** | [client-c] | Official |
| **Elixir** | [client-elixir] | Unofficial |
| **Go** | [client-go] | Official |
| **Java** | [client-java] | Official |
| **.Net** | [client-dotnet] | Official |
| **Node** | [client-node] | Official |
| **Rust** | [client-rust] | Unofficial |

## License

See: [LICENSE](LICENSE)

[TigerBeetle]: https://github.com/tigerbeetledb/tigerbeetle
[client-c]:https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/c
[client-go]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/go
[client-node]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/node
[client-java]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/java
[client-dotnet]: https://github.com/tigerbeetledb/tigerbeetle/tree/main/src/clients/dotnet
[client-elixir]: https://github.com/rbino/tigerbeetlex
[client-rust]: https://github.com/ZetaNumbers/tigerbeetle-rs
