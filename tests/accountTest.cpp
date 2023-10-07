#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <tb_client.hpp>

TEST_CASE("Account Test") {
  SUBCASE("Default Values") {
    tigerbeetle::account<1> accounts = tigerbeetle::make_account<1>();
    REQUIRE(accounts.size() == 1);
    tigerbeetle::account<1>::value_type default_value{};
    REQUIRE(accounts.at(0).id == default_value.id);
  }

  SUBCASE("Id") {
    tigerbeetle::account<1> accounts = tigerbeetle::make_account<1>();
    accounts.at(0).id = 100;
    REQUIRE(accounts.at(0).id == 100);
    accounts.at(0).id = 200;
    REQUIRE(accounts.at(0).id == 200);
  }

  SUBCASE("IdLong") {
    tigerbeetle::account<1> accounts = tigerbeetle::make_account<1>();
    accounts.at(0).id = 100;
    REQUIRE(accounts.at(0).id == 100);
    accounts.at(0).id = 0;
    REQUIRE(accounts.at(0).id == 0);
  }

  SUBCASE("IdAsBytes") {
    tigerbeetle::account<1> accounts = tigerbeetle::make_account<1>();
    uint8_t ids[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6};
    for (auto id : ids) {
      accounts.at(0).id = id;
      REQUIRE(id == accounts.at(0).id);
    }
  }

  SUBCASE("Account Comparison") {
    tigerbeetle::account<1> account1 = tigerbeetle::make_account<1>();
    tigerbeetle::account<1> account2 = tigerbeetle::make_account<1>();

    REQUIRE(account1.at(0).id == account2.at(0).id);

    account1.at(0).id = 100;
    REQUIRE(account1.at(0).id != account2.at(0).id);

    account2.at(0).id = 100;
    REQUIRE(account1.at(0).id == account2.at(0).id);
  }

  SUBCASE("Account Array") {
    constexpr std::size_t NumAccounts = 5;
    tigerbeetle::account<NumAccounts> accounts =
        tigerbeetle::make_account<NumAccounts>();

    REQUIRE(accounts.size() == NumAccounts);

    for (std::size_t i = 0; i < NumAccounts; ++i) {
      accounts.at(i).id = static_cast<tigerbeetle::tb_uint128_t>(i + 1);
    }

    for (std::size_t i = 0; i < NumAccounts; ++i) {
      REQUIRE(accounts.at(i).id ==
              static_cast<tigerbeetle::tb_uint128_t>(i + 1));
    }
  }

  SUBCASE("Multiple Accounts") {
    constexpr std::size_t NumAccounts = 10;
    tigerbeetle::account<NumAccounts> accounts =
        tigerbeetle::make_account<NumAccounts>();

    REQUIRE(accounts.size() == NumAccounts);

    for (std::size_t i = 0; i < NumAccounts; ++i) {
      accounts.at(i).id = static_cast<tigerbeetle::tb_uint128_t>(i + 1);
    }

    for (std::size_t i = 0; i < NumAccounts; ++i) {
      for (std::size_t j = i + 1; j < NumAccounts; ++j) {
        REQUIRE(accounts.at(i).id != accounts.at(j).id);
      }
    }
  }

  SUBCASE("Account Array Edge Case") {
    constexpr std::size_t MaxAccounts = 1000;
    tigerbeetle::account<MaxAccounts> accounts =
        tigerbeetle::make_account<MaxAccounts>();

    REQUIRE(accounts.size() == MaxAccounts);

    for (std::size_t i = 0; i < MaxAccounts; ++i) {
      accounts.at(i).id = static_cast<tigerbeetle::tb_uint128_t>(i + 1);
      REQUIRE(accounts.at(i).id ==
              static_cast<tigerbeetle::tb_uint128_t>(i + 1));
    }
  }
}