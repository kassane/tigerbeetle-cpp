#include <tb_client.hpp>

auto main() -> int
{
    fmt::println("TigerBeetle C Sample");
    fmt::println("Connecting...");

    asio::io_context io;
    tb::Client client(io, "localhost", 8080);
    io.run();
}