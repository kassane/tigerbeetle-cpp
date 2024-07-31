#include <cassert>
#include <cstdlib>
#include <fmt/format.h>
#include <tb_client.hpp>

namespace tb = tigerbeetle;

int main() {
  auto address = [&]() -> std::string {
    const char *env_address = std::getenv("TB_ADDRESS");
    if (env_address == nullptr) {
      return "3001";
    }
    std::string_view env_address_view(env_address);
    return std::string(env_address_view);
  }();

  try {
    fmt::println("TigerBeetle C++ - Multiple Two Phase [Sample]\n");

    fmt::println("Connecting...");

    tb::Client client(address);

    if (client.currentStatus() != tb::TB_STATUS_SUCCESS) {
      fmt::println(stderr, "Failed to initialize tb_client");
      return EXIT_FAILURE;
    }

    // Create two accounts
    tb::account<2> accounts;

    std::memset(accounts.data(), 0, accounts.size() * sizeof(tb::tb_account_t));

    accounts.at(0).id = 1;
    accounts.at(0).code = 1;
    accounts.at(0).ledger = 1;

    accounts.at(1).id = 2;
    accounts.at(1).code = 1;
    accounts.at(1).ledger = 1;

    tb::CompletionContext ctx{};
    tb::tb_packet_t packet{};

    packet.operation =
        tb::TB_OPERATION_CREATE_ACCOUNTS; // The operation to be performed.
    packet.data = accounts.data();        // The data to be sent.
    packet.data_size = accounts.size() * sizeof(tb::tb_account_t); //
    packet.user_data = &ctx;          // User-defined context.
    packet.status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

    fmt::println("Creating accounts...");

    client.send_request(packet, &ctx);

    if (packet.status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::println(stderr, "Error calling create_accounts (ret={})",
                   packet.status);
      return EXIT_FAILURE;
    }

    if (ctx.size != 0) {
      // Checking for errors creating the accounts:
      tb::tb_create_accounts_result_t *results =
          reinterpret_cast<tb::tb_create_accounts_result_t *>(ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_create_accounts_result_t);
      fmt::println("create_account results:");
      for (int i = 0; i < results_len; i++) {
        fmt::println("index={}, ret={}", results[i].index, results[i].result);
      }
      return EXIT_FAILURE;
    }

    fmt::println("Accounts created successfully");

    // Start five pending transfers
    tb::transfer<5> transfers;

    std::memset(transfers.data(), 0,
                transfers.size() * sizeof(tb::tb_transfer_t));

    transfers.at(0).id = 1;
    transfers.at(0).debit_account_id = 1;
    transfers.at(0).credit_account_id = 2;
    transfers.at(0).code = 1;
    transfers.at(0).ledger = 1;
    transfers.at(0).amount = 100;
    transfers.at(0).flags = tb::TB_TRANSFER_PENDING;

    transfers.at(1).id = 2;
    transfers.at(1).debit_account_id = 1;
    transfers.at(1).credit_account_id = 2;
    transfers.at(1).code = 1;
    transfers.at(1).ledger = 1;
    transfers.at(1).amount = 200;
    transfers.at(1).flags = tb::TB_TRANSFER_PENDING;

    transfers.at(2).id = 3;
    transfers.at(2).debit_account_id = 1;
    transfers.at(2).credit_account_id = 2;
    transfers.at(2).code = 1;
    transfers.at(2).ledger = 1;
    transfers.at(2).amount = 300;
    transfers.at(2).flags = tb::TB_TRANSFER_PENDING;

    transfers.at(3).id = 4;
    transfers.at(3).debit_account_id = 1;
    transfers.at(3).credit_account_id = 2;
    transfers.at(3).code = 1;
    transfers.at(3).ledger = 1;
    transfers.at(3).amount = 400;
    transfers.at(3).flags = tb::TB_TRANSFER_PENDING;

    transfers.at(4).id = 5;
    transfers.at(4).debit_account_id = 1;
    transfers.at(4).credit_account_id = 2;
    transfers.at(4).code = 1;
    transfers.at(4).ledger = 1;
    transfers.at(4).amount = 500;
    transfers.at(4).flags = tb::TB_TRANSFER_PENDING;

    packet.operation =
        tb::TB_OPERATION_CREATE_TRANSFERS; // The operation to be performed.
    packet.data = transfers.data();        // The data to be sent.
    packet.data_size = transfers.size() * sizeof(tb::tb_transfer_t); //
    packet.user_data = &ctx;          // User-defined context.
    packet.status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

    fmt::println("Creating transfers...");

    client.send_request(packet, &ctx);

    if (packet.status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::println(stderr, "Error calling create_transfers (ret={})",
                   packet.status);
      return EXIT_FAILURE;
    }

    if (ctx.size != 0) {
      // Checking for errors creating the transfers:
      tb::tb_create_transfers_result_t *results =
          reinterpret_cast<tb::tb_create_transfers_result_t *>(
              ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_create_transfers_result_t);
      fmt::println("create_transfers results:");
      for (int i = 0; i < results_len; i++) {
        fmt::println("index={}, ret={}", results[i].index, results[i].result);
      }
      return EXIT_FAILURE;
    }

    // Validate accounts pending and posted debits/credits before finishing the
    // two-phase transfer
    tb::accountID<2> ids = {1, 2};
    std::memset(accounts.data(), 0, accounts.size() * sizeof(tb::tb_account_t));

    packet.operation = tb::TB_OPERATION_LOOKUP_ACCOUNTS;
    packet.data = ids.data();
    packet.data_size = sizeof(tb::tb_uint128_t) * ids.size();
    packet.user_data = &ctx;
    packet.status = tb::TB_PACKET_OK;

    client.send_request(packet, &ctx);

    if (packet.status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::println(stderr, "Error calling lookup_accounts (ret={})",
                   packet.status);
      return EXIT_FAILURE;
    }

    if (ctx.size != 0) {
      // Validate the accounts' pending and posted debits/credits:
      tb::tb_account_t *results =
          reinterpret_cast<tb::tb_account_t *>(ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_account_t);
      fmt::println("{} Account(s) found", results_len);
      fmt::println("============================================");

      for (int i = 0; i < results_len; i++) {
        if (results[i].id == 1) {
          assert(results[i].debits_posted == 0);
          assert(results[i].credits_posted == 0);
          assert(results[i].debits_pending == 1500);
          assert(results[i].credits_pending == 0);
        } else if (results[i].id == 2) {
          assert(results[i].debits_posted == 0);
          assert(results[i].credits_posted == 0);
          assert(results[i].debits_pending == 0);
          assert(results[i].credits_pending == 1500);
        } else {
          fmt::println(stderr, "Unexpected account: {}", results[i].id);
          return EXIT_FAILURE;
        }
      }
    }

    // Create a 6th transfer posting the 1st transfer
    std::memset(transfers.data(), 0,
                transfers.size() * sizeof(tb::tb_transfer_t));
    transfers.at(0).id = 6;
    transfers.at(0).pending_id = 1;
    transfers.at(0).debit_account_id = 1;
    transfers.at(0).credit_account_id = 2;
    transfers.at(0).code = 1;
    transfers.at(0).ledger = 1;
    transfers.at(0).amount = 100;
    transfers.at(0).flags = tb::TB_TRANSFER_POST_PENDING_TRANSFER;

    packet.operation =
        tb::TB_OPERATION_CREATE_TRANSFERS; // The operation to be performed.
    packet.data = transfers.data();        // The data to be sent.
    packet.data_size = transfers.size() * sizeof(tb::tb_transfer_t); //
    packet.user_data = &ctx;          // User-defined context.
    packet.status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

    fmt::println("Creating transfers...");

    client.send_request(packet, &ctx);

    if (packet.status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::println(stderr, "Error calling create_transfers (ret={})",
                   packet.status);
      return EXIT_FAILURE;
    }

    if (ctx.size != 0) {
      // Checking for errors creating the transfers:
      tb::tb_create_transfers_result_t *results =
          reinterpret_cast<tb::tb_create_transfers_result_t *>(
              ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_create_transfers_result_t);
      fmt::println("create_transfers results:");
      for (int i = 0; i < results_len; i++) {
        fmt::println("index={}, ret={}", results[i].index, results[i].result);
      }

    } else {
      return EXIT_FAILURE;
    }

    // Validate accounts pending and posted debits/credits after finishing the
    // two-phase transfer
    std::memset(ids.data(), 0, ids.size() * sizeof(tb::tb_uint128_t));
    ids.at(0) = 1;
    ids.at(1) = 2;

    packet.operation = tb::TB_OPERATION_LOOKUP_ACCOUNTS;
    packet.data = ids.data();
    packet.data_size = sizeof(tb::tb_uint128_t) * ids.size();
    packet.user_data = &ctx;
    packet.status = tb::TB_PACKET_OK;

    client.send_request(packet, &ctx);

    if (packet.status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      fmt::println(stderr, "Error calling lookup_accounts (ret={})",
                   packet.status);
      return EXIT_FAILURE;
    }

    if (ctx.size != 0) {
      // Validate the accounts' pending and posted debits/credits:
      tb::tb_account_t *results =
          reinterpret_cast<tb::tb_account_t *>(ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_account_t);
      fmt::println("{} Account(s) found", results_len);
      fmt::println("============================================");

      for (int i = 0; i < results_len; i++) {
        if (results[i].id == 1) {
          assert(results[i].debits_posted == 100);
          assert(results[i].credits_posted == 0);
          assert(results[i].debits_pending == 1400);
          assert(results[i].credits_pending == 0);
        } else if (results[i].id == 2) {
          assert(results[i].debits_posted == 0);
          assert(results[i].credits_posted == 100);
          assert(results[i].debits_pending == 0);
          assert(results[i].credits_pending == 1400);
        } else {
          fmt::println(stderr, "Unexpected account: {}", results[i].id);
          return EXIT_FAILURE;
        }
      }
    }

    fmt::println("Multiple two-phase transfers completed successfully");
  } catch (std::exception &e) {
    fmt::println(stderr, "Exception occurred: {}", e.what());
    return EXIT_FAILURE;
  }

  return 0;
}
