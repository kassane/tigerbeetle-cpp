// Based:
// https://github.com/tigerbeetledb/tigerbeetle/blob/main/src/clients/c/samples/main.c

#include <tb_client.hpp>

namespace tb = TigerBeetle;

auto main() -> int {
  fmt::println("TigerBeetle C++ Sample");
  fmt::println("Connecting...");

  asio::io_context io;
  tb::CompletionContext ctx;

  tb::Client client(io, ctx, "localhost:3000");
  auto accounts = client.make_accounts<2>();
  std::memset(&accounts, 0, client.accounts_size());

  // ////////////////////////////////////////////////////////////
  // // Submitting a batch of accounts:                        //
  // ////////////////////////////////////////////////////////////

  accounts.at(0).id = 1;
  accounts.at(0).code = 2;
  accounts.at(0).ledger = 777;

  accounts.at(1).id = 2;
  accounts.at(1).code = 2;
  accounts.at(1).ledger = 777;

  client.account_init(accounts);
  auto packet = client.get_packet();
  auto packet_list = client.get_packet_list();
  auto packets_pool = client.get_packets_pool();
  auto ctx_ = client.get_context();

  // ////////////////////////////////////////////////////////////
  // // Submitting multiple batches of transfers:              //
  // ////////////////////////////////////////////////////////////

  client.transfers(accounts);

  // ////////////////////////////////////////////////////////////
  // // Looking up accounts:                                   //
  // ////////////////////////////////////////////////////////////

  fmt::println("Looking up accounts ...\n");
  tb_uint128_t ids[accounts.size()] = {accounts[0].id, accounts[1].id};

  // Acquiring a packet for this request:
  packet = client.acquire_packet(&packets_pool);
  packet->operation = TB_OPERATION_LOOKUP_ACCOUNTS;
  packet->data = ids;
  packet->data_size = sizeof(tb_uint128_t) * accounts.size();
  packet->user_data = &ctx;
  packet->status = TB_PACKET_OK;

  packet_list->head = packet.get();
  packet_list->tail = packet.get();
  client.send_request(client.get_client(), packet_list, ctx_);

  if (packet->status != TB_PACKET_OK) {
    // Checking if the request failed:
    fmt::println("Error calling lookup_accounts (ret={})", packet->status);
    exit(-1);
  }

  // Releasing the packet, so it can be used in a next request.
  client.release_packet(&packets_pool, packet.get());

  if (ctx.size == 0) {
    fmt::println("No accounts found");
    exit(-1);
  } else {
    // Printing the account's balance:
    tb_account_t *results =
        reinterpret_cast<tb_account_t *>(ctx_->reply.data());
    int results_len = ctx.size / sizeof(tb_account_t);
    fmt::println("{} Account(s) found\n", results_len);
    fmt::println("============================================\n");

    for (int i = 0; i < results_len; i++) {
      fmt::println("id={}\n", (long)results[i].id);
      fmt::println("debits_posted={}\n", results[i].debits_posted);
      fmt::println("credits_posted={}\n", results[i].credits_posted);
      fmt::println("\n");
    }
  }

  io.run();
}