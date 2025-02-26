#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fmt/format.h>
#include <tb_client.hpp>

inline long long get_time_ms() {
  auto now = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {

  fmt::println("TigerBeetle C++ Sample");
  fmt::println("Connecting...");

  auto address = [&]() -> std::string {
    const char *env_address = std::getenv("TB_ADDRESS");
    if (env_address == nullptr) {
      return "3001";
    }
    std::string_view env_address_view(env_address);
    return std::string(env_address_view);
  }();

  tigerbeetle::Client client(address);

  if (client.initStatus() != tigerbeetle::TB_INIT_SUCCESS) {
    fmt::println("Failed to initialize tb_client");
    return EXIT_FAILURE;
  }

  tigerbeetle::CompletionContext ctx{};
  tigerbeetle::tb_packet_t packet{};

  ////////////////////////////////////////////////////////////
  // Submitting a batch of accounts:                        //
  ////////////////////////////////////////////////////////////

  static constexpr size_t ACCOUNTS_LEN = 2;
  static constexpr size_t ACCOUNTS_SIZE =
      sizeof(tigerbeetle::tb_account_t) * ACCOUNTS_LEN;
  // tigerbeetle::tb_account_t accounts[ACCOUNTS_LEN];
  auto accounts = tigerbeetle::make_account<ACCOUNTS_LEN>();

  // Zeroing the memory, so we don't have to initialize every field.
  memset(&accounts, 0, ACCOUNTS_SIZE);

  accounts.at(0).id = 1;
  accounts.at(0).code = 2;
  accounts.at(0).ledger = 777;

  accounts.at(1).id = 2;
  accounts.at(1).code = 2;
  accounts.at(1).ledger = 777;

  packet.operation =
      tigerbeetle::TB_OPERATION_CREATE_ACCOUNTS; // The operation to be
                                                 // performed.
  packet.data = accounts.data();                 // The data to be sent.
  packet.data_size = ACCOUNTS_SIZE;              //
  packet.user_data = &ctx;                       // User-defined context.
  packet.status =
      tigerbeetle::TB_PACKET_OK; // Will be set when the reply arrives.

  fmt::print("Creating accounts...\n");

  client.send_request(packet, &ctx);

  if (packet.status != tigerbeetle::TB_PACKET_OK) {
    // Checking if the request failed:
    fmt::print("Error calling create_accounts (ret={})\n", packet.status);
    return EXIT_FAILURE;
  }

  if (ctx.size != 0) {
    // Checking for errors creating the accounts:
    auto results = reinterpret_cast<tigerbeetle::tb_create_accounts_result_t *>(
        ctx.reply.data());
    int results_len =
        ctx.size / sizeof(tigerbeetle::tb_create_accounts_result_t);
    fmt::print("create_account results:\n");
    for (int i = 0; i < results_len; i++) {
      fmt::print("index={}, ret={}\n", results[i].index, results[i].result);
    }
    return EXIT_FAILURE;
  }

  fmt::print("Accounts created successfully\n");

  ////////////////////////////////////////////////////////////
  // Submitting multiple batches of transfers:              //
  ////////////////////////////////////////////////////////////

  fmt::print("Creating transfers...\n");
  static constexpr auto MAX_BATCHES = 100;
  static constexpr auto TRANSFERS_PER_BATCH =
      ((tigerbeetle::MAX_MESSAGE_SIZE) / sizeof(tigerbeetle::tb_transfer_t));
  static constexpr auto TRANSFERS_SIZE =
      (sizeof(tigerbeetle::tb_transfer_t) * TRANSFERS_PER_BATCH);
  long max_latency_ms = 0;
  long total_time_ms = 0;
  for (int i = 0; i < MAX_BATCHES; i++) {
    auto transfers = tigerbeetle::make_transfer<TRANSFERS_PER_BATCH>();

    // Zeroing the memory, so we don't have to initialize every field.
    std::memset(transfers.data(), 0, TRANSFERS_SIZE);

    std::size_t baseID = i * TRANSFERS_PER_BATCH + 1;
    std::size_t j = 0;

    std::transform(transfers.begin(), transfers.end(), transfers.begin(),
                   [&](tigerbeetle::tb_transfer_t &transfer) {
                     transfer.id = baseID + j++;
                     transfer.debit_account_id = accounts.at(0).id;
                     transfer.credit_account_id = accounts.at(1).id;
                     transfer.code = 2;
                     transfer.ledger = 777;
                     transfer.amount = 1;
                     return transfer;
                   });

    packet.operation =
        tigerbeetle::TB_OPERATION_CREATE_TRANSFERS;   // The operation to be
                                                      // performed.
    packet.data = transfers.data();                   // The data to be sent.
    packet.data_size = tigerbeetle::MAX_MESSAGE_SIZE; //
    packet.user_data = &ctx;                          // User-defined context.
    packet.status =
        tigerbeetle::TB_PACKET_OK; // Will be set when the reply arrives.

    long long now = get_time_ms();

    client.send_request(packet, &ctx);

    long elapsed_ms = get_time_ms() - now;
    if (elapsed_ms > max_latency_ms)
      max_latency_ms = elapsed_ms;
    total_time_ms += elapsed_ms;

    if (packet.status != tigerbeetle::TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::print("Error calling create_transfers (ret={})\n", packet.status);
      return EXIT_FAILURE;
    }

    if (ctx.size != 0) {
      // Checking for errors creating the accounts:
      auto results =
          reinterpret_cast<tigerbeetle::tb_create_transfers_result_t *>(
              ctx.reply.data());
      int results_len =
          ctx.size / sizeof(tigerbeetle::tb_create_transfers_result_t);
      fmt::print("create_transfers results:\n");
      for (int idx = 0; idx < results_len; idx++) {
        fmt::print("index={}, ret={}\n", results[idx].index,
                   results[idx].result);
      }
      return EXIT_FAILURE;
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
  tigerbeetle::accountID<ACCOUNTS_LEN> ids = {accounts.at(0).id,
                                              accounts.at(1).id};

  packet.operation = tigerbeetle::TB_OPERATION_LOOKUP_ACCOUNTS;
  packet.data = ids.data();
  packet.data_size = sizeof(tigerbeetle::tb_uint128_t) * ACCOUNTS_LEN;
  packet.user_data = &ctx;
  packet.status = tigerbeetle::TB_PACKET_OK;

  client.send_request(packet, &ctx);

  if (packet.status != tigerbeetle::TB_PACKET_OK) {
    // Checking if the request failed:
    fmt::print("Error calling lookup_accounts (ret={})", packet.status);
    return EXIT_FAILURE;
  }

  if (ctx.size == 0) {
    fmt::println("No accounts found");
    return EXIT_FAILURE;
  } else {
    // Printing the account's balance:
    auto results =
        reinterpret_cast<tigerbeetle::tb_account_t *>(ctx.reply.data());
    int results_len = ctx.size / sizeof(tigerbeetle::tb_account_t);
    fmt::print("{} Account(s) found\n", results_len);
    fmt::print("============================================\n");

    std::for_each(results, results + results_len,
                  [&](const tigerbeetle::tb_account_t &result) {
                    fmt::println("id={}", static_cast<std::size_t>(result.id));
                    fmt::println("debits_posted={}", result.debits_posted);
                    fmt::println("credits_posted={}", result.credits_posted);
                  });
  }

  return EXIT_SUCCESS;
}
