// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.

#define BOOST_TEST_NO_MAIN LiquibookTest
#include <boost/test/unit_test.hpp>

#include "ut_utils.h"

namespace liquibook {

using simple::EfviSimpleOrder;
typedef EfviFillCheck<EfviSimpleOrder*> SimpleFillCheck;

OrderConditions IOC(oc_immediate_or_cancel);
OrderConditions FOK(oc_all_or_none | oc_immediate_or_cancel);

BOOST_AUTO_TEST_CASE(TestIocBidNoMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 100);
  EfviSimpleOrder bid0(true,  1250, 100);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  {
    SimpleFillCheck fc0(&bid0, 0, 0, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    //SimpleFillCheck fc3(&ask0, 0, 0);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, false, false, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIocBidPartialMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 100);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Partial Match - will cancel order
  {
    SimpleFillCheck fc0(&bid0, 100, 125000, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 100, 125000);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, false, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIocBidFullMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 400);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 400));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  {
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, true, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIocBidMultiMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 400);
  EfviSimpleOrder bid0(true,  1251, 500);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 400));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  {
    SimpleFillCheck fc0(&bid0, 500, 625100, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 1250 * 400);
    SimpleFillCheck fc4(&ask1, 100, 1251 * 100);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, true, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokBidNoMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 100);
  EfviSimpleOrder bid0(true,  1250, 100);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  {
    SimpleFillCheck fc0(&bid0, 0, 0, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    //SimpleFillCheck fc3(&ask0, 0, 0);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, false, false, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokBidPartialMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 100);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Partial Match - will not fill efviAnd will cancel order
  {
    SimpleFillCheck fc0(&bid0, 0, 0, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, false, false, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokBidFullMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 400);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 400));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  {
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, true, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokBidMultiMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 400);
  EfviSimpleOrder bid0(true,  1251, 500);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1250, 1, 400));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  {
    SimpleFillCheck fc0(&bid0, 500, 625100, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 1250 * 400);
    SimpleFillCheck fc4(&ask1, 100, 1251 * 100);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, true, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIocAskNoMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 100);
  EfviSimpleOrder bid0(true,  1250, 100);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  {
    // SimpleFillCheck fc0(&bid0, 0, 0);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, false, false, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIocAskPartialMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 300);
  EfviSimpleOrder bid0(true,  1250, 100);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Partial Match - will cancel order
  {
    SimpleFillCheck fc0(&bid0, 100, 125000);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 100, 125000, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, false, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIocAskFullMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 300);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 300));
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full match
  {
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, true, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIocAskMultiMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1249, 400);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 300));
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full match
  {
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 100, 1249 * 100);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 499900, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, true, IOC));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokAskNoMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 100);
  EfviSimpleOrder bid0(true,  1250, 100);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  {
    // SimpleFillCheck fc0(&bid0, 0, 0);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, false, false, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokAskPartialMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 300);
  EfviSimpleOrder bid0(true,  1250, 100);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Partial Match - will not fill efviAnd will cancel order
  {
    SimpleFillCheck fc0(&bid0, 0, 0);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, false, false, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokAskFullMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1250, 300);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 300));
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full Match
  {
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, true, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestFokAskMultiMatch)
{
  EfviSimpleOrderBook order_book;
  EfviSimpleOrder ask2(false, 1252, 100);
  EfviSimpleOrder ask1(false, 1251, 100);
  EfviSimpleOrder ask0(false, 1249, 400);
  EfviSimpleOrder bid0(true,  1250, 300);
  EfviSimpleOrder bid1(true,  1249, 100);
  EfviSimpleOrder bid2(true,  1248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  EfviDepthCheck<EfviSimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 300));
  BOOST_CHECK(dc.verify_bid(1249, 1, 100));
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Full match
  {
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 100, 1249 * 100);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 499900, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, true, FOK));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1248, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

} // Namespace


