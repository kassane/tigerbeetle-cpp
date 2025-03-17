#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fmt/format.h>
#include <tb_client.hpp>

inline auto get_time_ms() noexcept {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  fmt::print("TigerBeetle C++ Sample\n");
  fmt::print("Connecting...\n");

  auto address = []() -> std::string_view {
    if (const char *env_address = std::getenv("TB_ADDRESS"); env_address) {
      return env_address;
    }
    return "3001";
  }();

  tigerbeetle::Client client(address);

  if (client.initStatus() != tigerbeetle::TB_INIT_SUCCESS) {
    fmt::println("Failed to initialize tb_client");
    return EXIT_FAILURE;
  }

  tigerbeetle::CompletionContext ctx{};
  tigerbeetle::tb_packet_t packet{};

  ////////////////////////////////////////////////////////////
  // Submitting a batch of accounts
  ////////////////////////////////////////////////////////////

  static constexpr size_t ACCOUNTS_LEN = 2;
  auto accounts = tigerbeetle::make_account<ACCOUNTS_LEN>();

  // Fully initialize all fields in declaration order
  accounts[0] = tigerbeetle::tb_account_t{.id = 1,
                                          .debits_pending = 0,
                                          .debits_posted = 0,
                                          .credits_pending = 0,
                                          .credits_posted = 0,
                                          .user_data_128 = 0,
                                          .user_data_64 = 0,
                                          .user_data_32 = 0,
                                          .reserved = 0,
                                          .ledger = 777,
                                          .code = 2,
                                          .flags = 0,
                                          .timestamp = 0};
  accounts[1] = tigerbeetle::tb_account_t{.id = 2,
                                          .debits_pending = 0,
                                          .debits_posted = 0,
                                          .credits_pending = 0,
                                          .credits_posted = 0,
                                          .user_data_128 = 0,
                                          .user_data_64 = 0,
                                          .user_data_32 = 0,
                                          .reserved = 0,
                                          .ledger = 777,
                                          .code = 2,
                                          .flags = 0,
                                          .timestamp = 0};

  packet.operation = tigerbeetle::TB_OPERATION_CREATE_ACCOUNTS;
  packet.data = accounts.data();
  packet.data_size = sizeof(tigerbeetle::tb_account_t) * ACCOUNTS_LEN;
  packet.user_data = &ctx;
  packet.status = tigerbeetle::TB_PACKET_OK;

  fmt::print("Creating accounts...\n");
  client.send_request(packet, &ctx);

  if (packet.status != tigerbeetle::TB_PACKET_OK) {
    fmt::print("Error calling create_accounts (ret={})\n", packet.status);
    return EXIT_FAILURE;
  }

  if (ctx.size != 0) {
    std::span<const tigerbeetle::tb_create_accounts_result_t> results(
        reinterpret_cast<const tigerbeetle::tb_create_accounts_result_t *>(
            ctx.reply.data()),
        ctx.size / sizeof(tigerbeetle::tb_create_accounts_result_t));
    fmt::print("create_account results:\n");
    for (const auto &result : results) {
      fmt::print("index={}, ret={}\n", result.index,
                 static_cast<int>(result.result));
    }
    return EXIT_FAILURE;
  }

  fmt::print("Accounts created successfully\n");

  ////////////////////////////////////////////////////////////
  // Submitting multiple batches of transfers
  ////////////////////////////////////////////////////////////

  fmt::print("Creating transfers...\n");
  static constexpr auto MAX_BATCHES = 100;
  static constexpr auto TRANSFERS_PER_BATCH =
      tigerbeetle::MAX_MESSAGE_SIZE / sizeof(tigerbeetle::tb_transfer_t);
  long max_latency_ms = 0;
  long total_time_ms = 0;

  for (int i = 0; i < MAX_BATCHES; ++i) {
    auto transfers = tigerbeetle::make_transfer<TRANSFERS_PER_BATCH>();

    // Initialize transfers using ranges, fixing lambda to modify in-place
    size_t baseID = i * TRANSFERS_PER_BATCH + 1;
    size_t j = 0;
    std::ranges::for_each(transfers, [&](auto &transfer) {
      transfer.id = baseID + j++;
      transfer.debit_account_id = accounts[0].id;
      transfer.credit_account_id = accounts[1].id;
      transfer.amount = 1;
      transfer.pending_id = 0;
      transfer.user_data_128 = 0;
      transfer.user_data_64 = 0;
      transfer.user_data_32 = 0;
      transfer.timeout = 0;
      transfer.ledger = 777;
      transfer.code = 2;
      transfer.flags = 0;
      transfer.timestamp = 0;
    });

    packet.operation = tigerbeetle::TB_OPERATION_CREATE_TRANSFERS;
    packet.data = transfers.data();
    packet.data_size = sizeof(tigerbeetle::tb_transfer_t) * TRANSFERS_PER_BATCH;
    packet.user_data = &ctx;
    packet.status = tigerbeetle::TB_PACKET_OK;

    auto start = std::chrono::steady_clock::now();
    client.send_request(packet, &ctx);
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start)
                          .count();

    if (elapsed_ms > max_latency_ms)
      max_latency_ms = elapsed_ms;
    total_time_ms += elapsed_ms;

    if (packet.status != tigerbeetle::TB_PACKET_OK) {
      fmt::print("Error calling create_transfers (ret={})\n", packet.status);
      return EXIT_FAILURE;
    }

    if (ctx.size != 0) {
      std::span<const tigerbeetle::tb_create_transfers_result_t> results(
          reinterpret_cast<const tigerbeetle::tb_create_transfers_result_t *>(
              ctx.reply.data()),
          ctx.size / sizeof(tigerbeetle::tb_create_transfers_result_t));
      fmt::print("create_transfers results:\n");
      for (const auto &result : results) {
        fmt::print("index={}, ret={}\n", result.index,
                   static_cast<int>(result.result));
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
  // Looking up accounts
  ////////////////////////////////////////////////////////////

  fmt::print("Looking up accounts ...\n");
  std::array<tigerbeetle::tb_uint128_t, ACCOUNTS_LEN> ids = {accounts[0].id,
                                                             accounts[1].id};

  packet.operation = tigerbeetle::TB_OPERATION_LOOKUP_ACCOUNTS;
  packet.data = ids.data();
  packet.data_size = sizeof(tigerbeetle::tb_uint128_t) * ACCOUNTS_LEN;
  packet.user_data = &ctx;
  packet.status = tigerbeetle::TB_PACKET_OK;

  client.send_request(packet, &ctx);

  if (packet.status != tigerbeetle::TB_PACKET_OK) {
    fmt::print("Error calling lookup_accounts (ret={})\n", packet.status);
    return EXIT_FAILURE;
  }

  if (ctx.size == 0) {
    fmt::print("No accounts found\n");
    return EXIT_FAILURE;
  } else {
    std::span<const tigerbeetle::tb_account_t> results(
        reinterpret_cast<const tigerbeetle::tb_account_t *>(ctx.reply.data()),
        ctx.size / sizeof(tigerbeetle::tb_account_t));
    fmt::print("{} Account(s) found\n", results.size());
    fmt::print("============================================\n");

    std::ranges::for_each(results, [](const auto &result) {
      fmt::print("id={}\n", static_cast<std::size_t>(result.id));
      fmt::print("debits_posted={}\n", result.debits_posted);
      fmt::print("credits_posted={}\n", result.credits_posted);
    });
  }

  return EXIT_SUCCESS;
}