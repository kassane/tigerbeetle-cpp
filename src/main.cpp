#include <chrono>
#include <tb_client.hpp>

namespace tb = tigerbeetle;

static constexpr size_t MAX_BATCHES = 100;
static constexpr size_t TRANSFERS_PER_BATCH =
    tb::MAX_MESSAGE_SIZE / sizeof(tb::tb_transfer_t);

auto main() -> int {
  fmt::print(fmt::fg(fmt::color::green_yellow) | fmt::emphasis::bold,
             "TigerBeetle C++ Sample\n\n");
  fmt::print("Connecting...\n");
  tb::tb_client_t client{};
  tb::tb_packet_list_t packets_pool;
  std::string address = "127.0.0.1:3000";

  tb::TB_STATUS status = tb::tb_client_init(
      &client,         // Output client.
      &packets_pool,   // Output packet list.
      0,               // Cluster ID.
      address.c_str(), // Cluster addresses.
      address.size(),  //
      32, // MaxConcurrency, could be 1, since it's a single-threaded example.
      0,  // No need for a global context.
      &tb::on_completion // Completion callback.
  );

  if (status != tb::TB_STATUS_SUCCESS) {
    fmt::print(stderr, fmt::fg(fmt::color::crimson) | fmt::emphasis::bold,
               "Failed to initialize tb::tb_client (ret={})\n",
               fmt::underlying(status));
    std::exit(-1);
  }

  tb::CompletionContext ctx;
  std::shared_ptr<tb::tb_packet_t> packet;
  tb::tb_packet_list_t packet_list;

  ////////////////////////////////////////////////////////////
  // Submitting a batch of accounts:                        //
  ////////////////////////////////////////////////////////////

  tb::account<2> accounts;

  // Zeroing the memory, so we don't have to initialize every field.
  std::memset(&accounts, 0, accounts.size());

  accounts.at(0).id = 1;
  accounts.at(0).code = 2;
  accounts.at(0).ledger = 777;

  accounts.at(1).id = 2;
  accounts.at(1).code = 2;
  accounts.at(1).ledger = 777;

  // Acquiring a packet for this request:
  packet = acquire_packet(packets_pool);
  packet->operation =
      tb::TB_OPERATION_CREATE_ACCOUNTS; // The operation to be performed.
  packet->data = accounts.data();       // The data to be sent.
  packet->data_size = accounts.size();  //
  packet->user_data = &ctx;             // User-defined context.
  packet->status = tb::TB_PACKET_OK;    // Will be set when the reply arrives.

  fmt::print("Creating accounts...\n");

  packet_list.head = packet.get();
  packet_list.tail = packet.get();
  tb::send_request(client, packet_list, ctx);

  if (packet->status != tb::TB_PACKET_OK) {
    // Checking if the request failed:
    fmt::print(stderr, fmt::fg(fmt::color::crimson) | fmt::emphasis::bold,
               "Error calling create_accounts (ret={})\n", packet->status);
    std::exit(-1);
  }

  // Releasing the packet, so it can be used in a next request.
  tb::release_packet(packets_pool, packet);

  if (ctx.size != 0) {
    // Checking for errors creating the accounts:
    tb::tb_create_accounts_result_t *results =
        reinterpret_cast<tb::tb_create_accounts_result_t *>(ctx.reply.data());
    int results_len = ctx.size / sizeof(tb::tb_create_accounts_result_t);
    fmt::print("create_account results:\n");
    for (int i = 0; i < results_len; i++) {
      fmt::print("index={}, ret={}\n", results[i].index, results[i].result);
    }
    std::exit(-1);
  }

  fmt::print(fmt::fg(fmt::color::green) | fmt::emphasis::bold,
             "Accounts created successfully\n");

  ////////////////////////////////////////////////////////////
  // Submitting multiple batches of transfers:              //
  ////////////////////////////////////////////////////////////

  fmt::print("Creating transfers...\n");

  long max_latency_ms = 0;
  long total_time_ms = 0;
  for (std::size_t i = 0; i < MAX_BATCHES; i++) {
    tb::transfer<TRANSFERS_PER_BATCH> transfers;

    // Zeroing the memory, so we don't have to initialize every field.
    std::memset(transfers.data(), 0, transfers.size());

    for (size_t j = 0; j < TRANSFERS_PER_BATCH; j++) {
      transfers.at(j).id = j + 1 + (i * TRANSFERS_PER_BATCH);
      transfers.at(j).debit_account_id = accounts.at(0).id;
      transfers.at(j).credit_account_id = accounts.at(1).id;
      transfers.at(j).code = 2;
      transfers.at(j).ledger = 777;
      transfers.at(j).amount = 1;
    }

    // Acquiring a packet for this request:
    packet = acquire_packet(packets_pool);
    packet->operation =
        tb::TB_OPERATION_CREATE_TRANSFERS;    // The operation to be performed.
    packet->data = transfers.data();          // The data to be sent.
    packet->data_size = tb::MAX_MESSAGE_SIZE; //
    packet->user_data = &ctx;                 // User-defined context.
    packet->status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

    long long now = std::chrono::system_clock::now().time_since_epoch().count();

    packet_list.head = packet.get();
    packet_list.tail = packet.get();
    tb::send_request(client, packet_list, ctx);

    long elapsed_ms =
        std::chrono::system_clock::now().time_since_epoch().count() - now;
    if (elapsed_ms > max_latency_ms)
      max_latency_ms = elapsed_ms;
    total_time_ms += elapsed_ms;

    if (packet->status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::print(stderr, fmt::fg(fmt::color::crimson) | fmt::emphasis::bold,
                 "Error calling create_transfers (ret={})\n", packet->status);
      std::exit(-1);
    }

    // Releasing the packet, so it can be used in a next request.
    tb::release_packet(packets_pool, packet);

    if (ctx.size != 0) {
      // Checking for errors creating the accounts:
      tb::tb_create_transfers_result_t *results =
          reinterpret_cast<tb::tb_create_transfers_result_t *>(
              ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_create_transfers_result_t);
      fmt::print("create_transfers results:\n");
      for (int i = 0; i < results_len; i++) {
        fmt::print("index={}, ret={}\n", results[i].index, results[i].result);
      }
      std::exit(-1);
    }
  }

  fmt::print(fmt::fg(fmt::color::green) | fmt::emphasis::bold,
             "Transfers created successfully\n");
  fmt::println("============================================");

  fmt::print("{} transfers per second\n",
             (MAX_BATCHES * TRANSFERS_PER_BATCH * 1000) / total_time_ms);
  fmt::print("create_transfers max p100 latency per {} transfers = {}ms\n",
             TRANSFERS_PER_BATCH, max_latency_ms);
  fmt::print("total {} transfers in {}ms\n", MAX_BATCHES * TRANSFERS_PER_BATCH,
             total_time_ms);
  fmt::print("\n");

  ////////////////////////////////////////////////////////////
  // Looking up accounts:                                   //
  ////////////////////////////////////////////////////////////

  fmt::print("Looking up accounts ...\n");
  tb::accountID<2> ids = {accounts.at(0).id, accounts.at(1).id};

  // Acquiring a packet for this request:
  packet = acquire_packet(packets_pool);
  packet->operation = tb::TB_OPERATION_LOOKUP_ACCOUNTS;
  packet->data = ids.data();
  packet->data_size = sizeof(tb::tb_uint128_t) * accounts.size();
  packet->user_data = &ctx;
  packet->status = tb::TB_PACKET_OK;

  packet_list.head = packet.get();
  packet_list.tail = packet.get();
  tb::send_request(client, packet_list, ctx);

  if (packet->status != tb::TB_PACKET_OK) {
    // Checking if the request failed:
    fmt::print(stderr, fmt::fg(fmt::color::crimson) | fmt::emphasis::bold,
               "Error calling lookup_accounts (ret={})", packet->status);
    std::exit(-1);
  }

  // Releasing the packet, so it can be used in a next request.
  tb::release_packet(packets_pool, packet);

  if (ctx.size == 0) {
    fmt::print(fmt::fg(fmt::color::yellow) | fmt::emphasis::bold,
               "No accounts found!\n");
    std::exit(-1);
  } else {
    // Printing the account's balance:
    tb::tb_account_t *results =
        reinterpret_cast<tb::tb_account_t *>(ctx.reply.data());
    int results_len = ctx.size / sizeof(tb::tb_account_t);
    fmt::print(fmt::fg(fmt::color::green) | fmt::emphasis::bold,
               "{} Account(s) found\n", results_len);
    fmt::println("============================================");

    for (int i = 0; i < results_len; i++) {
      fmt::print("id={}\n", static_cast<long>(results[i].id));
      fmt::print("debits_posted={}\n", results[i].debits_posted);
      fmt::println("credits_posted={}", results[i].credits_posted);
    }
  }

  // Cleanup:
  tb::tb_client_deinit(client);
}
