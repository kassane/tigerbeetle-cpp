#include <algorithm>
#include <cassert>
#include <tb_client.hpp>
#include <tb_logger.hpp>

namespace tb = tigerbeetle;

int main() {
  std::string address = "127.0.0.1:3001";
  tb::Logger log;
  try {

    fmt::print(fmt::fg(fmt::color::green_yellow) | fmt::emphasis::bold,
               "TigerBeetle C++ - Two Phase [Sample]\n\n");

    log.trace("Connecting...");

    tb::Client client(
        0,       // Cluster ID.
        address, // Server address.
        32, // MaxConcurrency, could be 1 since it's a single-threaded example.
        0,  // No need for a global context.
        tb::on_completion // Completion callback.
    );

    if (client.currentStatus() != tb::TB_STATUS_SUCCESS) {
      log.error("Failed to initialize tb_client");
      return -1;
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
    tb::tb_packet_t *packet = nullptr;

    // Acquiring a packet for this request:
    if (client.acquire_packet(&packet) != tb::TB_PACKET_ACQUIRE_OK) {
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

    client.send_request(packet, &ctx);

    if (packet->status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      log.error(fmt::format("Error calling create_accounts (ret={})",
                            packet->status));
      return -1;
    }

    // Releasing the packet, so it can be used in a next request.
    client.release_packet(&packet);

    if (ctx.size != 0) {
      // Checking for errors creating the accounts:
      tb::tb_create_accounts_result_t *results =
          reinterpret_cast<tb::tb_create_accounts_result_t *>(ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_create_accounts_result_t);
      log.info("create_account results:");
      for (int i = 0; i < results_len; i++) {
        log.info(fmt::format("index={}, ret={}", results[i].index,
                             results[i].result));
      }
      return -1;
    }

    log.info("Accounts created successfully");

    // Start a pending transfer
    tb::transfer<1> transfers;

    std::memset(transfers.data(), 0,
                transfers.size() * sizeof(tb::tb_transfer_t));

    transfers.at(0).id = 1;
    transfers.at(0).debit_account_id = 1;
    transfers.at(0).credit_account_id = 2;
    transfers.at(0).code = 1;
    transfers.at(0).ledger = 1;
    transfers.at(0).amount = 500;
    transfers.at(0).flags = tb::TB_TRANSFER_PENDING;

    // Acquiring a packet for this request:
    if (client.acquire_packet(&packet) != tb::TB_PACKET_ACQUIRE_OK) {
      log.error("Too many concurrent packets.");
      return -1;
    }

    packet->operation =
        tb::TB_OPERATION_CREATE_TRANSFERS; // The operation to be performed.
    packet->data = transfers.data();       // The data to be sent.
    packet->data_size = transfers.size() * sizeof(tb::tb_transfer_t); //
    packet->user_data = &ctx;          // User-defined context.
    packet->status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

    log.trace("Creating transfers...");

    client.send_request(packet, &ctx);

    if (packet->status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      log.error(fmt::format("Error calling create_transfers (ret={})",
                            packet->status));
      return -1;
    }

    // Releasing the packet, so it can be used in the next request.
    client.release_packet(&packet);

    if (ctx.size != 0) {
      // Checking for errors creating the transfers:
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

    // Validate accounts pending and posted debits/credits before finishing the
    // two-phase transfer
    tb::accountID<2> ids = {1, 2};
    std::memset(accounts.data(), 0, accounts.size() * sizeof(tb::tb_account_t));

    // Acquiring a packet for this request:
    if (client.acquire_packet(&packet) != tb::TB_PACKET_ACQUIRE_OK) {
      log.error("Too many concurrent packets.");
      return -1;
    }

    packet->operation = tb::TB_OPERATION_LOOKUP_ACCOUNTS;
    packet->data = ids.data();
    packet->data_size = sizeof(tb::tb_uint128_t) * ids.size();
    packet->user_data = &ctx;
    packet->status = tb::TB_PACKET_OK;

    client.send_request(packet, &ctx);

    if (packet->status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      log.error(fmt::format("Error calling lookup_accounts (ret={})",
                            packet->status));
      return -1;
    }

    // Releasing the packet, so it can be used in a next request.
    client.release_packet(&packet);

    if (ctx.size != 0) {
      // Validate the accounts' pending and posted debits/credits:
      tb::tb_account_t *results =
          reinterpret_cast<tb::tb_account_t *>(ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_account_t);
      log.info(fmt::format("{} Account(s) found", results_len));
      fmt::println("============================================");

      for (int i = 0; i < results_len; i++) {
        if (results[i].id == 1) {
          assert(results[i].debits_posted == 0);
          assert(results[i].credits_posted == 0);
          assert(results[i].debits_pending == 500);
          assert(results[i].credits_pending == 0);
        } else if (results[i].id == 2) {
          assert(results[i].debits_posted == 0);
          assert(results[i].credits_posted == 0);
          assert(results[i].debits_pending == 0);
          assert(results[i].credits_pending == 500);
        } else {
          log.error(fmt::format("Unexpected account: {}", results[i].id));
          return -1;
        }
      }
    }

    // Create a second transfer simply posting the first transfer
    std::memset(transfers.data(), 0,
                transfers.size() * sizeof(tb::tb_transfer_t));
    transfers.at(0).id = 2;
    transfers.at(0).pending_id = 1;
    transfers.at(0).debit_account_id = 1;
    transfers.at(0).credit_account_id = 2;
    transfers.at(0).code = 1;
    transfers.at(0).ledger = 1;
    transfers.at(0).amount = 500;
    transfers.at(0).flags = tb::TB_TRANSFER_POST_PENDING_TRANSFER;

    // Acquiring a packet for this request:
    if (client.acquire_packet(&packet) != tb::TB_PACKET_ACQUIRE_OK) {
      log.error("Too many concurrent packets.");
      return -1;
    }

    packet->operation =
        tb::TB_OPERATION_CREATE_TRANSFERS; // The operation to be performed.
    packet->data = transfers.data();       // The data to be sent.
    packet->data_size = transfers.size() * sizeof(tb::tb_transfer_t); //
    packet->user_data = &ctx;          // User-defined context.
    packet->status = tb::TB_PACKET_OK; // Will be set when the reply arrives.

    log.trace("Creating transfers...");

    client.send_request(packet, &ctx);

    if (packet->status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      log.error(fmt::format("Error calling create_transfers (ret={})",
                            packet->status));
      return -1;
    }

    // Releasing the packet, so it can be used in the next request.
    client.release_packet(&packet);

    if (ctx.size != 0) {
      // Checking for errors creating the transfers:
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

    // Validate accounts pending and posted debits/credits after finishing the
    // two-phase transfer
    std::memset(ids.data(), 0, ids.size() * sizeof(tb::tb_uint128_t));
    ids.at(0) = 1;
    ids.at(1) = 2;

    // Acquiring a packet for this request:
    if (client.acquire_packet(&packet) != tb::TB_PACKET_ACQUIRE_OK) {
      log.error("Too many concurrent packets.");
      return -1;
    }

    packet->operation = tb::TB_OPERATION_LOOKUP_ACCOUNTS;
    packet->data = ids.data();
    packet->data_size = sizeof(tb::tb_uint128_t) * ids.size();
    packet->user_data = &ctx;
    packet->status = tb::TB_PACKET_OK;

    client.send_request(packet, &ctx);

    if (packet->status != tb::TB_PACKET_OK) {
      // Checking if the request failed:
      log.error(fmt::format("Error calling lookup_accounts (ret={})",
                            packet->status));
      return -1;
    }

    // Releasing the packet, so it can be used in a next request.
    client.release_packet(&packet);

    if (ctx.size != 0) {
      // Validate the accounts' pending and posted debits/credits:
      tb::tb_account_t *results =
          reinterpret_cast<tb::tb_account_t *>(ctx.reply.data());
      int results_len = ctx.size / sizeof(tb::tb_account_t);
      log.info(fmt::format("{} Account(s) found", results_len));
      fmt::println("============================================");

      for (int i = 0; i < results_len; i++) {
        if (results[i].id == 1) {
          assert(results[i].debits_posted == 500);
          assert(results[i].credits_posted == 0);
          assert(results[i].debits_pending == 0);
          assert(results[i].credits_pending == 0);
        } else if (results[i].id == 2) {
          assert(results[i].debits_posted == 0);
          assert(results[i].credits_posted == 500);
          assert(results[i].debits_pending == 0);
          assert(results[i].credits_pending == 0);
        } else {
          log.error(fmt::format("Unexpected account: {}", results[i].id));
          return -1;
        }
      }
    }

    log.info("Two-phase transfer completed successfully");
  } catch (std::exception &e) {
    log.error(fmt::format("Exception occurred: {}", e.what()));
    return -1;
  }
}
