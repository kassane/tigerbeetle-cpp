#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <numeric>
#include <tb_client.hpp>

TEST_CASE("Transfer Test") {
  SUBCASE("Single Transfer") {
    tigerbeetle::transfer<1> transfers = tigerbeetle::make_transfer<1>();
    REQUIRE(transfers.size() == 1);

    // Set up transfer details
    transfers.at(0).id = 123;
    transfers.at(0).debit_account_id = 456;
    transfers.at(0).credit_account_id = 789;
    transfers.at(0).amount = 1000;

    // Check transfer details
    REQUIRE(transfers.at(0).id == 123);
    REQUIRE(transfers.at(0).debit_account_id == 456);
    REQUIRE(transfers.at(0).credit_account_id == 789);
    REQUIRE(transfers.at(0).amount == 1000);

    // Limit tests
    REQUIRE(transfers.at(0).id <=
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max());
    REQUIRE(transfers.at(0).debit_account_id <=
            std::numeric_limits<uint64_t>::max());
    REQUIRE(transfers.at(0).credit_account_id <=
            std::numeric_limits<uint64_t>::max());
    REQUIRE(transfers.at(0).amount <= std::numeric_limits<uint64_t>::max());
  }

  SUBCASE("Multiple Transfers") {
    constexpr std::size_t NumTransfers = 3;
    tigerbeetle::transfer<NumTransfers> transfers =
        tigerbeetle::make_transfer<NumTransfers>();
    REQUIRE(transfers.size() == NumTransfers);

    // Set up transfer details for each transfer
    for (std::size_t i = 0; i < NumTransfers; ++i) {
      transfers.at(i).id = static_cast<tigerbeetle::tb_uint128_t>(i + 1);
      transfers.at(i).debit_account_id =
          static_cast<tigerbeetle::tb_uint128_t>(i + 100);
      transfers.at(i).credit_account_id =
          static_cast<tigerbeetle::tb_uint128_t>(i + 200);
      transfers.at(i).amount =
          static_cast<tigerbeetle::tb_uint128_t>(1000 * (i + 1));
    }

    // Check transfer details for each transfer
    for (std::size_t i = 0; i < NumTransfers; ++i) {
      REQUIRE(transfers.at(i).id ==
              static_cast<tigerbeetle::tb_uint128_t>(i + 1));
      REQUIRE(transfers.at(i).debit_account_id ==
              static_cast<tigerbeetle::tb_uint128_t>(i + 100));
      REQUIRE(transfers.at(i).credit_account_id ==
              static_cast<tigerbeetle::tb_uint128_t>(i + 200));
      REQUIRE(transfers.at(i).amount ==
              static_cast<tigerbeetle::tb_uint128_t>(1000 * (i + 1)));

      // Limit tests
      REQUIRE(transfers.at(i).id <=
              std::numeric_limits<tigerbeetle::tb_uint128_t>::max());
      REQUIRE(transfers.at(i).debit_account_id <=
              std::numeric_limits<uint64_t>::max());
      REQUIRE(transfers.at(i).credit_account_id <=
              std::numeric_limits<uint64_t>::max());
      REQUIRE(transfers.at(i).amount <= std::numeric_limits<uint64_t>::max());
    }
  }

  SUBCASE("Transfer with numeric Limits") {
    tigerbeetle::transfer<1> transfers = tigerbeetle::make_transfer<1>();
    REQUIRE(transfers.size() == 1);

    // Set up transfer details with limits
    transfers.at(0).id =
        std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1;
    transfers.at(0).debit_account_id =
        std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1;
    transfers.at(0).credit_account_id =
        std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1;
    transfers.at(0).amount =
        std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1;

    // Check transfer details
    REQUIRE(transfers.at(0).id ==
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1);
    REQUIRE(transfers.at(0).debit_account_id ==
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1);
    REQUIRE(transfers.at(0).credit_account_id ==
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1);
    REQUIRE(transfers.at(0).amount ==
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max() + 1);

    // Limit tests
    REQUIRE(transfers.at(0).id <=
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max());
    REQUIRE(transfers.at(0).debit_account_id <=
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max());
    REQUIRE(transfers.at(0).credit_account_id <=
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max());
    REQUIRE(transfers.at(0).amount <=
            std::numeric_limits<tigerbeetle::tb_uint128_t>::max());
  }

  SUBCASE("Concept tb types") {
    static_assert(tigerbeetle::tb_integral<tigerbeetle::tb_uint128_t>,
                  "tb_uint128_t is TBInteger");
    //     static_assert(tigerbeetle::tb_integral<tigerbeetle::tb_transfer_t>,
    //     "tb_transfer_t is not a TBInteger"); // get compile time error
    static_assert(tigerbeetle::tb_same<tigerbeetle::tb_account_t>,
                  "tb_account_t is not a TBInteger");
    static_assert(tigerbeetle::tb_same<tigerbeetle::tb_transfer_t>,
                  "tb_transfer_t is not a TBInteger");
  }
}
