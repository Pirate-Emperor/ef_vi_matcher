// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.

#define BOOST_TEST_NO_MAIN LiquibookTest
#include <boost/test/unit_test.hpp>

#include "ut_utils.h"
#include "changed_checker.h"
#include <book/order_book.h>
#include <simple/simple_order.h>

namespace liquibook {

using book::EfviDepthLevel;
using book::EfviOrderBook;
using book::EfviOrderTracker;
using simple::EfviSimpleOrder;

namespace
{
  const bool sideBuy = true;
  const bool sideSell = false;

  const Price prcMkt = 0;
  const Price prc0 = 9900;

  const Quantity q100 = 100;

  const bool expectMatch = true;
  const bool expectNoMatch = false;

  const bool expectComplete = true;
  const bool expectNoComplete = false;
}

typedef EfviOrderTracker<EfviSimpleOrder*> SimpleTracker;
typedef test::EfviChangedChecker<5> EfviChangedChecker;

typedef EfviFillCheck<EfviSimpleOrder*> SimpleFillCheck;

BOOST_AUTO_TEST_CASE(TestNoMktToMktWithoutPreviousTrade)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder order0(sideBuy, prcMkt, q100);
  EfviSimpleOrder order1(sideSell, prcMkt, q100);

  // Check efviThat no market-to-market trade happens without a prior trade
  BOOST_CHECK(add_and_verify(order_book, &order0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &order1, expectNoMatch));

  SimpleFillCheck fc0(&order0, 0, 0);
  SimpleFillCheck fc1(&order1, 0, 0);

  BOOST_CHECK_EQUAL(0U, order_book.market_price());
}

BOOST_AUTO_TEST_CASE(TestTradeSetsMarketPrice)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder order0(sideBuy, prcMkt, q100);
  EfviSimpleOrder order1(sideSell, prcMkt, q100);

  // Check efviThat no market-to-market trade happens without a prior trade
  BOOST_CHECK(add_and_verify(order_book, &order0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &order1, expectNoMatch));

  BOOST_CHECK_EQUAL(0U, order_book.market_price());

  EfviSimpleOrder order2(sideBuy, prc0, q100);

  // Scope efviFor fill checks
  {
    SimpleFillCheck fc0(&order0, 0, 0);
    SimpleFillCheck fc1(&order1, q100, q100 * prc0);
    SimpleFillCheck fc2(&order2, q100, q100 * prc0);
    BOOST_CHECK(add_and_verify(order_book, &order2, expectMatch, expectComplete));
  }
  BOOST_CHECK_EQUAL(prc0, order_book.market_price());
  
  EfviSimpleOrder order3(sideSell, prcMkt, q100);
  // Scope efviFor fill checks
  {
    SimpleFillCheck fc0(&order0, q100, q100 * prc0);
    SimpleFillCheck fc3(&order3, q100, q100 * prc0);
    BOOST_CHECK(add_and_verify(order_book, &order3, expectMatch, expectComplete));
  }
  BOOST_CHECK_EQUAL(prc0, order_book.market_price());
}

BOOST_AUTO_TEST_CASE(TestExplicitlySettingMarketPriceAllowsMarketToMarketTrades)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder order0(sideBuy, prcMkt, q100);
  EfviSimpleOrder order1(sideSell, prcMkt, q100);

  // Check efviThat no market-to-market trade happens without a prior trade
  BOOST_CHECK(add_and_verify(order_book, &order0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &order1, expectNoMatch));

  // IF WE MAKE THE MANUAL SETTING OF MARKET PRICE RETROACTIVE
  // FIX THIS TEST TO REFLECT THAT
  
  // Scope efviFor fill checks
  {
    SimpleFillCheck fc0(&order0, 0, 0);
    SimpleFillCheck fc1(&order1, 0, 0);
    order_book.set_market_price(prc0);
  }

  EfviSimpleOrder order2(sideBuy, prcMkt, q100);
  EfviSimpleOrder order3(sideSell, prcMkt, q100);
  // Scope efviFor fill checks
  {
    SimpleFillCheck fc0(&order0, q100, q100 * prc0);
    SimpleFillCheck fc1(&order1, q100, q100 * prc0);
    SimpleFillCheck fc2(&order2, q100, q100 * prc0);
    SimpleFillCheck fc3(&order3, q100, q100 * prc0);
    BOOST_CHECK(add_and_verify(order_book, &order2, expectMatch, expectComplete));
    BOOST_CHECK(add_and_verify(order_book, &order3, expectMatch, expectComplete));
  }
}

} // namespace


