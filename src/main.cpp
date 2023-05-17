#include <tb_client.hpp>

using namespace tigerbeetle;

auto main() -> int {
  fmt::print("TigerBeetle C++ Sample\n");
  fmt::print("Connecting...\n");
  tb_client_t client = nullptr;
  tb_packet_list_t packets_pool;
  std::string address = "127.0.0.1:3000";

  TB_STATUS status = tb_client_init(
      &client,         // Output client.
      &packets_pool,   // Output packet list.
      0,               // Cluster ID.
      address.c_str(), // Cluster addresses.
      address.size(),  //
      32, // MaxConcurrency, could be 1, since it's a single-threaded example.
      0,  // No need for a global context.
      &on_completion // Completion callback.
  );

  if (status != TB_STATUS_SUCCESS) {
    fmt::print("Failed to initialize tb_client\n");
    std::exit(-1);
  }

  CompletionContext ctx;
  completion_context_init(ctx);

  std::shared_ptr<tb_packet_t> packet;
  tb_packet_list_t packet_list;

  ////////////////////////////////////////////////////////////
  // Submitting a batch of accounts:                        //
  ////////////////////////////////////////////////////////////

  account<2> accounts;

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
      TB_OPERATION_CREATE_ACCOUNTS;    // The operation to be performed.
  packet->data = accounts.data();      // The data to be sent.
  packet->data_size = accounts.size(); //
  packet->user_data = &ctx;            // User-defined context.
  packet->status = TB_PACKET_OK;       // Will be set when the reply arrives.

  fmt::print("Creating accounts...\n");

  packet_list.head = packet.get();
  packet_list.tail = packet.get();
  send_request(client, packet_list, ctx);

  if (packet->status != TB_PACKET_OK) {
    // Checking if the request failed:
    fmt::print("Error calling create_accounts (ret={})\n", packet->status);
    std::exit(-1);
  }

  // Releasing the packet, so it can be used in a next request.
  release_packet(packets_pool, packet);

  if (ctx.size != 0) {
    // Checking for errors creating the accounts:
    tb_create_accounts_result_t *results =
        reinterpret_cast<tb_create_accounts_result_t *>(ctx.reply.data());
    int results_len = ctx.size / sizeof(tb_create_accounts_result_t);
    fmt::print("create_account results:\n");
    for (int i = 0; i < results_len; i++) {
      fmt::print("index={}, ret={}\n", results[i].index, results[i].result);
    }
    std::exit(-1);
  }

  fmt::print("Accounts created successfully\n");

  ////////////////////////////////////////////////////////////
  // Submitting multiple batches of transfers:              //
  ////////////////////////////////////////////////////////////

  fmt::print("Creating transfers...\n");

  long max_latency_ms = 0;
  long total_time_ms = 0;
  for (std::size_t i = 0; i < MAX_BATCHES; i++) {
    transfer<TRANSFERS_PER_BATCH> transfers;

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
        TB_OPERATION_CREATE_TRANSFERS;    // The operation to be performed.
    packet->data = transfers.data();      // The data to be sent.
    packet->data_size = MAX_MESSAGE_SIZE; //
    packet->user_data = &ctx;             // User-defined context.
    packet->status = TB_PACKET_OK;        // Will be set when the reply arrives.

    long long now = get_time_ms();

    packet_list.head = packet.get();
    packet_list.tail = packet.get();
    send_request(client, packet_list, ctx);

    long elapsed_ms = get_time_ms() - now;
    if (elapsed_ms > max_latency_ms)
      max_latency_ms = elapsed_ms;
    total_time_ms += elapsed_ms;

    if (packet->status != TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::print("Error calling create_transfers (ret={})\n", packet->status);
      std::exit(-1);
    }

    // Releasing the packet, so it can be used in a next request.
    release_packet(packets_pool, packet);

    if (ctx.size != 0) {
      // Checking for errors creating the accounts:
      tb_create_transfers_result_t *results =
          reinterpret_cast<tb_create_transfers_result_t *>(ctx.reply.data());
      int results_len = ctx.size / sizeof(tb_create_transfers_result_t);
      fmt::print("create_transfers results:\n");
      for (int i = 0; i < results_len; i++) {
        fmt::print("index={}, ret={}\n", results[i].index, results[i].result);
      }
      std::exit(-1);
    }
  }

  fmt::print("Transfers created successfully\n");
  fmt::print("============================================\n");

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
  accountID<2> ids = {accounts.at(0).id, accounts.at(1).id};

  // Acquiring a packet for this request:
  packet = acquire_packet(packets_pool);
  packet->operation = TB_OPERATION_LOOKUP_ACCOUNTS;
  packet->data = ids.data();
  packet->data_size = sizeof(tb_uint128_t) * accounts.size();
  packet->user_data = &ctx;
  packet->status = TB_PACKET_OK;

  packet_list.head = packet.get();
  packet_list.tail = packet.get();
  send_request(client, packet_list, ctx);

  if (packet->status != TB_PACKET_OK) {
    // Checking if the request failed:
    fmt::print("Error calling lookup_accounts (ret={})", packet->status);
    std::exit(-1);
  }

  // Releasing the packet, so it can be used in a next request.
  release_packet(packets_pool, packet);

  if (ctx.size == 0) {
    fmt::print("No accounts found");
    std::exit(-1);
  } else {
    // Printing the account's balance:
    tb_account_t *results = reinterpret_cast<tb_account_t *>(ctx.reply.data());
    int results_len = ctx.size / sizeof(tb_account_t);
    fmt::print("{} Account(s) found\n", results_len);
    fmt::print("============================================\n");

    for (int i = 0; i < results_len; i++) {
      fmt::print("id={}\n", static_cast<long>(results[i].id));
      fmt::print("debits_posted={}\n", results[i].debits_posted);
      fmt::print("credits_posted={}\n", results[i].credits_posted);
      fmt::print("\n");
    }
  }

  // Cleanup:
  completion_context_destroy(ctx);
  tb_client_deinit(client);
}
