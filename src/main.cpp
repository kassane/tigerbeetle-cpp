#include <chrono>
#include <tb_client.hpp>

namespace tb = tigerbeetle;

auto main() -> int {
  constexpr size_t MAX_BATCHES = 100;
  constexpr size_t TRANSFERS_PER_BATCH =
      tb::MAX_MESSAGE_SIZE / sizeof(tb::tb_transfer_t);

  tb::Logger log;

  fmt::print(fmt::fg(fmt::color::green_yellow) | fmt::emphasis::bold,
             "TigerBeetle C++ Sample\n\n");

  log.trace("Connecting...");
  tb::tb_client_t client{};
  std::string address = "127.0.0.1:3001";

  tb::TB_STATUS status = tb::tb_client_init(
      &client,         // Output client.
      0,               // Cluster ID.
      address.c_str(), // Cluster addresses.
      address.size(),  //
      32, // MaxConcurrency, could be 1, since it's a single-threaded example.
      0,  // No need for a global context.
      &tb::on_completion // Completion callback.
  );

  if (status != tb::TB_STATUS_SUCCESS) {
    log.error(fmt::format("Failed to initialize tb::tb_client (ret={})",
                          fmt::underlying(status)));
    return -1;
  }

  tb::CompletionContext ctx{};
  tb::tb_packet_t *packet = nullptr;

  ////////////////////////////////////////////////////////////
  // Submitting a batch of accounts:                        //
  ////////////////////////////////////////////////////////////

  tb::account<2> accounts;

  // Zeroing the memory, so we don't have to initialize every field.
  std::memset(accounts.data(), 0, accounts.size() * sizeof(tb::tb_account_t));

  accounts.at(0).id = 1;
  accounts.at(0).code = 2;
  accounts.at(0).ledger = 777;

  accounts.at(1).id = 2;
  accounts.at(1).code = 2;
  accounts.at(1).ledger = 777;

  // Acquiring a packet for this request:
  if (tb::tb_client_acquire_packet(client, &packet) != tb::TB_PACKET_ACQUIRE_OK) {
      log.error("Too many concurrent packets.");
      return -1;
  }

  packet->operation =
      tb::TB_OPERATION_CREATE_ACCOUNTS; // The operation to be performed.
  packet->data = accounts.data();       // The data to be sent.
  packet->data_size = accounts.size() * sizeof(tb::tb_account_t); //
  packet->user_data = &ctx;          // User-defined context.
  packet->status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

  log.trace("Creating accounts...");

 
  tb::send_request(client, packet, &ctx);

  if (packet->status != tb::TB_PACKET_OK) {
    // Checking if the request failed:
    log.error(
        fmt::format("Error calling create_accounts (ret={})", packet->status));
    return -1;
  }

  // Releasing the packet, so it can be used in a next request.
  tb::tb_client_release_packet(client, packet);

  if (ctx.size != 0) {
    // Checking for errors creating the accounts:
    tb::tb_create_accounts_result_t *results =
        reinterpret_cast<tb::tb_create_accounts_result_t *>(ctx.reply.data());
    int results_len = ctx.size / sizeof(tb::tb_create_accounts_result_t);
    log.info("create_account results:");
    for (int i = 0; i < results_len; i++) {
      log.info(
          fmt::format("index={}, ret={}", results[i].index, results[i].result));
    }
    return -1;
  }

  log.info("Accounts created successfully");

  ////////////////////////////////////////////////////////////
  // Submitting multiple batches of transfers:              //
  ////////////////////////////////////////////////////////////

  log.trace("Creating transfers...");

  std::size_t max_latency_ms = 0;
  std::size_t total_time_ms = 0;
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
  if (tb::tb_client_acquire_packet(client, &packet) != tb::TB_PACKET_ACQUIRE_OK) {
      log.error("Too many concurrent packets.");
      return -1;
  }

    packet->operation =
        tb::TB_OPERATION_CREATE_TRANSFERS;    // The operation to be performed.
    packet->data = transfers.data();          // The data to be sent.
    packet->data_size = tb::MAX_MESSAGE_SIZE; //
    packet->user_data = &ctx;                 // User-defined context.
    packet->status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

    std::size_t now =
        std::chrono::system_clock::now().time_since_epoch().count();

    tb::send_request(client, packet, &ctx);

    std::size_t elapsed_ms =
        std::chrono::system_clock::now().time_since_epoch().count() - now;
    if (elapsed_ms > max_latency_ms)
      max_latency_ms = elapsed_ms;

    total_time_ms += elapsed_ms;

    if (packet->status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      log.error(fmt::format("Error calling create_transfers (ret={})",
                            packet->status));
      return -1;
    }

    // Releasing the packet, so it can be used in a next request.
    tb::tb_client_release_packet(client, packet);

    if (ctx.size != 0) {
      // Checking for errors creating the accounts:
      tb::tb_create_transfers_result_t *results =
          reinterpret_cast<tb::tb_create_transfers_result_t *>(
              ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_create_transfers_result_t);
      log.info("create_transfers results:");
      for (int i = 0; i < results_len; i++) {
        log.info(fmt::format("index={}, ret={}", results[i].index,
                             results[i].result));
      }
      return -1;
    }
  }

  log.info("Transfers created successfully");
  fmt::println("============================================");

  log.trace(
      fmt::format("{} transfers per second",
                  (MAX_BATCHES * TRANSFERS_PER_BATCH * 1000) / total_time_ms));
  log.trace(
      fmt::format("create_transfers max p100 latency per {} transfers = {}ms",
                  TRANSFERS_PER_BATCH, max_latency_ms));
  log.trace(fmt::format("total {} transfers in {}ms",
                        MAX_BATCHES * TRANSFERS_PER_BATCH, total_time_ms));

  ////////////////////////////////////////////////////////////
  // Looking up accounts:                                   //
  ////////////////////////////////////////////////////////////

  log.info("Looking up accounts ...");
  tb::accountID<2> ids = {accounts.at(0).id, accounts.at(1).id};

  // Acquiring a packet for this request:
  if (tb::tb_client_acquire_packet(client, &packet) != tb::TB_PACKET_ACQUIRE_OK) {
      log.error("Too many concurrent packets.");
      return -1;
  }
  
  packet->operation = tb::TB_OPERATION_LOOKUP_ACCOUNTS;
  packet->data = ids.data();
  packet->data_size = sizeof(tb::tb_uint128_t) * accounts.size();
  packet->user_data = &ctx;
  packet->status = tb::TB_PACKET_OK;

  tb::send_request(client, packet, &ctx);

  if (packet->status != tb::TB_PACKET_OK) {
    // Checking if the request failed:
    log.error(
        fmt::format("Error calling lookup_accounts (ret={})", packet->status));
    return -1;
  }

  // Releasing the packet, so it can be used in a next request.
  tb::tb_client_release_packet(client, packet);

  if (ctx.size == 0) {
    log.warn("No accounts found!");
    return -1;
  }
  // Printing the account's balance:
  tb::tb_account_t *results =
      reinterpret_cast<tb::tb_account_t *>(ctx.reply.data());
  int results_len = ctx.size / sizeof(tb::tb_account_t);
  log.info(fmt::format("{} Account(s) found", results_len));
  fmt::println("============================================");

  for (int i = 0; i < results_len; i++) {
    log.trace(fmt::format("id={}", static_cast<std::size_t>(results[i].id)));
    log.trace(fmt::format("debits_posted={}", results[i].debits_posted));
    log.trace(fmt::format("credits_posted={}", results[i].credits_posted));
  }

  // Cleanup:
  tb::tb_client_deinit(client);
  return 0;
}
