/*
Copyright (c) 2023 Matheus Catarino França (matheus-catarino@hotmail.com)

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.
*/
#ifndef TB_CLIENT_HPP
#define TB_CLIENT_HPP
#pragma once
#include <array>
#include <concepts>
#include <cstdint>
#include <mutex>
#include <string>

namespace tigerbeetle {

#include <tb_client.h>

using tb_uint32_t = std::uint32_t;
using tb_uint64_t = std::uint64_t;
template <typename T>
concept tb_same =
    std::is_same_v<T, tb_uint128_t> || std::is_same_v<T, tb_transfer_t> ||
    std::is_same_v<T, tb_account_t>;

template <typename T>
concept tb_integral =
    std::is_integral_v<T> && sizeof(T) == sizeof(tb_uint128_t);

constexpr size_t MAX_MESSAGE_SIZE = (1024 * 1024) - 128;
template <std::size_t N> using accountID = std::array<tb_uint128_t, N>;
template <std::size_t N> using transferID = std::array<tb_uint128_t, N>;
template <std::size_t N> using transfer = std::array<tb_transfer_t, N>;
template <std::size_t N> using account = std::array<tb_account_t, N>;

template <std::size_t N>
concept AccountID = requires { typename accountID<N>; };
template <std::size_t N>
concept TransferID = requires { typename transferID<N>; };
template <std::size_t N>
concept TransferArray = requires { typename transfer<N>; };
template <std::size_t N>
concept AccountArray = requires { typename account<N>; };
template <std::size_t N>
  requires AccountID<N>
auto make_account() {
  return account<N>{};
}
template <std::size_t N>
  requires TransferArray<N>
auto make_transfer() {
  return transfer<N>{};
}

// Synchronization context between the callback and the main thread.
struct CompletionContext {
  std::array<uint8_t, MAX_MESSAGE_SIZE> reply;
  int size;
  bool completed;
  std::mutex mutex;
};

class Client {
public:
  Client(uint32_t cluster_id, const std::string &address,
         uint32_t packets_count, uintptr_t on_completion_ctx,
         void (*on_completion_fn)(uintptr_t, tb_client_t, tb_packet_t *,
                                  const uint8_t *, uint32_t))
      : client(nullptr) {
    status =
        tb_client_init(&client, cluster_id, address.c_str(), address.length(),
                       packets_count, on_completion_ctx, on_completion_fn);
  }

  Client(const Client &) = delete;
  Client &operator=(const Client &) = delete;

  Client(Client &&other) : client(nullptr) { std::swap(client, other.client); }

  Client &operator=(Client &&other) {
    if (this != &other) {
      destroy();
      std::swap(client, other.client);
    }
    return *this;
  }

  ~Client() { destroy(); }

  tb_client_t get() const { return client; }

  TB_PACKET_ACQUIRE_STATUS acquire_packet(tb_packet_t **packet) const {
    return tb_client_acquire_packet(client, packet);
  }

  TB_STATUS currentStatus() { return status; };

  void release_packet(tb_packet_t **packet) {
    tb_client_release_packet(client, *packet);
  }

  void send_request(tb_packet_t *packet, CompletionContext *ctx) {
    // Submits the request asynchronously:
    ctx->completed = false;
    {
      std::lock_guard<std::mutex> lock(ctx->mutex);
      tb_client_submit(client, packet);
    }
    while (true) {
      std::lock_guard<std::mutex> lock(ctx->mutex);
      if (ctx->completed) {
        break;
      }
      // Release the lock and wait for completion
    }
  }

private:
  void destroy() {
    if (client != nullptr) {
      tb_client_deinit(client);
      client = nullptr;
    }
  }

  tb_client_t client;
  TB_STATUS status;
};

inline void on_completion([[maybe_unused]] uintptr_t context,
                          [[maybe_unused]] tb_client_t client,
                          tb_packet_t *packet, const uint8_t *data,
                          uint32_t size) {
  auto ctx = static_cast<CompletionContext *>(packet->user_data);
  {
    std::lock_guard<std::mutex> lock(
        ctx->mutex); // Lock the mutex before accessing ctx members
    std::copy(data, data + size, ctx->reply.begin());
    ctx->size = size;
    ctx->completed = true;
  }
}
} // namespace tigerbeetle
#endif // TB_CLIENT_HPP