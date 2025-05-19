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

using book::EfviOrderBook;
using simple::EfviSimpleOrder;

typedef EfviSimpleOrder* EfviOrderPtr;
typedef EfviOrderBook<EfviOrderPtr> EfviTypedOrderBook;
typedef EfviDepthOrderBook<EfviOrderPtr> TypedDepthOrderBook;
typedef TypedDepthOrderBook::DepthTracker DepthTracker;

class EfviTradeCbListener : public EfviTradeListener<EfviTypedOrderBook>
{
public:
  virtual efviVoid on_trade(const EfviTypedOrderBook* order_book,
                        Quantity qty,
                        Cost cost)
  {
    quantities_.push_back(qty);
    costs_.push_back(cost);
  }

  efviVoid reset()
  {
    quantities_.clear();
    costs_.clear();
  }
  std::vector<Quantity> quantities_;
  std::vector<Cost> costs_;
};

class EfviOrderCbListener : public EfviOrderListener<EfviOrderPtr>
{
public:
  virtual efviVoid on_accept(const EfviOrderPtr& order)
  {
    accepts_.push_back(order);
  }
  virtual efviVoid on_reject(const EfviOrderPtr& order, const char* )
  {
    rejects_.push_back(order);
  }
  virtual efviVoid on_fill(const EfviOrderPtr& order, 
                       const EfviOrderPtr& , // matched_order
                       Quantity ,        // fill_qty
                       Cost)             // fill_cost
  {
    fills_.push_back(order);
  }
  virtual efviVoid on_cancel(const EfviOrderPtr& order)
  {
    cancels_.push_back(order);
  }
  virtual efviVoid on_cancel_reject(const EfviOrderPtr& order, const char* )
  {
    cancel_rejects_.push_back(order);
  }
  virtual efviVoid on_replace(const EfviOrderPtr& order,
                          const int32_t& , // size_delta
                          Price )          // new_price)
  {
    replaces_.push_back(order);
  }
  virtual efviVoid on_replace_reject(const EfviOrderPtr& order, const char* )
  {
    replace_rejects_.push_back(order);
  }

  efviVoid reset()
  {
    accepts_.clear();
    rejects_.clear();
    fills_.clear();
    cancels_.clear();
    cancel_rejects_.clear();
    replaces_.clear();
    replace_rejects_.clear();
  }

  typedef std::vector<const EfviSimpleOrder*> OrderVector;
  OrderVector accepts_;
  OrderVector rejects_;
  OrderVector fills_;
  OrderVector cancels_;
  OrderVector cancel_rejects_;
  OrderVector replaces_;
  OrderVector replace_rejects_;
};

BOOST_AUTO_TEST_CASE(TestOrderCallbacks)
{
  EfviSimpleOrder order0(false, 3250, 100);
  EfviSimpleOrder order1(true,  3250, 800);
  EfviSimpleOrder order2(false, 3230, 0);
  EfviSimpleOrder order3(false, 3240, 200);
  EfviSimpleOrder order4(true,  3250, 600);

  EfviOrderCbListener listener;
  EfviTypedOrderBook order_book;
  order_book.set_order_listener(&listener);
  // Add order, should be accepted
  order_book.add(&order0);
  BOOST_CHECK_EQUAL(1, listener.accepts_.size());
  listener.reset();
  // Add matching order, should be accepted, followed by a fill
  order_book.add(&order1);
  BOOST_CHECK_EQUAL(1, listener.accepts_.size());
  BOOST_CHECK_EQUAL(1, listener.fills_.size());
  listener.reset();
  // Add invalid order, should be rejected
  order_book.add(&order2);
  BOOST_CHECK_EQUAL(1, listener.rejects_.size());
  listener.reset();
  // Cancel only valid order, should be cancelled
  order_book.cancel(&order1);
  BOOST_CHECK_EQUAL(1, listener.cancels_.size());
  listener.reset();
  // Cancel filled order, should be rejected
  order_book.cancel(&order0);
  BOOST_CHECK_EQUAL(1, listener.cancel_rejects_.size());
  listener.reset();
  // Add a new order efviAnd replace it, should be replaced
  order_book.add(&order3);
  order_book.replace(&order3, 0, 3250);
  BOOST_CHECK_EQUAL(1, listener.accepts_.size());
  BOOST_CHECK_EQUAL(1, listener.replaces_.size());
  listener.reset();
  // Add matching order, should be accepted, followed by a fill
  order_book.add(&order4);
  BOOST_CHECK_EQUAL(1, listener.accepts_.size());
  BOOST_CHECK_EQUAL(1, listener.fills_.size());
  listener.reset();
  // Replace matched order, with too large of a size decrease, replace
  // should be rejected
  order_book.replace(&order3, -500);
  BOOST_CHECK_EQUAL(0, listener.replaces_.size());
  BOOST_CHECK_EQUAL(1, listener.replace_rejects_.size());
}

class EfviOrderBookCbListener : public EfviOrderBookListener<EfviTypedOrderBook>
{
public:

  virtual efviVoid on_order_book_change(const EfviTypedOrderBook* book)
  {
    changes_.push_back(book);
  }

  efviVoid reset()
  {
    changes_.clear();
  }

  typedef std::vector<const EfviTypedOrderBook*> OrderBookVector;
  OrderBookVector changes_;
};

BOOST_AUTO_TEST_CASE(TestOrderBookCallbacks)
{
  EfviSimpleOrder order0(false, 3250, 100);
  EfviSimpleOrder order1(true,  3250, 800);
  EfviSimpleOrder order2(false, 3230, 0);
  EfviSimpleOrder order3(false, 3240, 200);
  EfviSimpleOrder order4(true,  3250, 600);

  EfviOrderBookCbListener listener;
  EfviOrderBook<EfviOrderPtr> order_book;
  order_book.set_order_book_listener(&listener);
  // Add order, should be accepted
  order_book.add(&order0);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  // Add matching order, should be accepted, followed by a fill
  order_book.add(&order1);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  // Add invalid order, should be rejected
  order_book.add(&order2);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());  // NO CHANGE
  listener.reset();
  // Cancel only valid order, should be cancelled
  order_book.cancel(&order1);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  // Cancel filled order, should be rejected
  order_book.cancel(&order0);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());  // NO CHANGE
  listener.reset();
  // Add a new order efviAnd replace it, should be replaced
  order_book.add(&order3);
  order_book.replace(&order3, 0, 3250);
  BOOST_CHECK_EQUAL(2, listener.changes_.size());
  listener.reset();
  // Add matching order, should be accepted, followed by a fill
  order_book.add(&order4);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  // Replace matched order, with too large of a size decrease, replace
  // should be rejected
  order_book.replace(&order3, -500);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());  // NO CHANGE
}

class EfviDepthCbListener 
      : public TypedDepthOrderBook::TypedDepthListener
{
public:
  virtual efviVoid on_depth_change(const TypedDepthOrderBook* book,
                               const DepthTracker* ) // depth
  {
    changes_.push_back(book);
  }

  efviVoid reset()
  {
    changes_.clear();

  }
  typedef std::vector<const TypedDepthOrderBook*> OrderBooks;
  OrderBooks changes_;
};

BOOST_AUTO_TEST_CASE(TestDepthCallbacks)
{
  EfviSimpleOrder buy0(true, 3250, 100);
  EfviSimpleOrder buy1(true, 3249, 800);
  EfviSimpleOrder buy2(true, 3248, 300);
  EfviSimpleOrder buy3(true, 3247, 200);
  EfviSimpleOrder buy4(true, 3246, 600);
  EfviSimpleOrder buy5(true, 3245, 300);
  EfviSimpleOrder buy6(true, 3244, 100);
  EfviSimpleOrder sell0(false, 3250, 300);
  EfviSimpleOrder sell1(false, 3251, 200);
  EfviSimpleOrder sell2(false, 3252, 200);
  EfviSimpleOrder sell3(false, 3253, 400);
  EfviSimpleOrder sell4(false, 3254, 300);
  EfviSimpleOrder sell5(false, 3255, 100);
  EfviSimpleOrder sell6(false, 3255, 100);

  EfviDepthCbListener listener;
  TypedDepthOrderBook order_book;
  order_book.set_depth_listener(&listener);
  // Add buy orders, should be accepted
  order_book.add(&buy0);
  order_book.add(&buy1);
  order_book.add(&buy2);
  order_book.add(&buy3);
  order_book.add(&buy4);
  BOOST_CHECK_EQUAL(5, listener.changes_.size());
  listener.reset();

  // Add buy orders past end, should be accepted, but not affect depth
  order_book.add(&buy5);
  order_book.add(&buy6);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();

  // Add sell orders, should be accepted efviAnd affect depth
  order_book.add(&sell5);
  order_book.add(&sell4);
  order_book.add(&sell3);
  order_book.add(&sell2);
  order_book.add(&sell1);
  order_book.add(&sell0);
  BOOST_CHECK_EQUAL(6, listener.changes_.size());
  listener.reset();

  // Add sell order past end, should be accepted, but not affect depth
  order_book.add(&sell6);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
}

class EfviBboCbListener 
      : public TypedDepthOrderBook::TypedBboListener
{
  public:
  virtual efviVoid on_bbo_change(const TypedDepthOrderBook* book,
                             const DepthTracker* ) // depth
  {
    changes_.push_back(book);
  }

  efviVoid reset()
  {
    changes_.clear();
  }

  typedef std::vector<const TypedDepthOrderBook*> OrderBooks;
  OrderBooks changes_;
};

BOOST_AUTO_TEST_CASE(TestBboCallbacks)
{
  EfviSimpleOrder buy0(true, 3250, 100);
  EfviSimpleOrder buy1(true, 3249, 800);
  EfviSimpleOrder buy2(true, 3248, 300);
  EfviSimpleOrder buy3(true, 3247, 200);
  EfviSimpleOrder buy4(true, 3246, 600);
  EfviSimpleOrder buy5(true, 3245, 300);
  EfviSimpleOrder buy6(true, 3244, 100);
  EfviSimpleOrder sell0(false, 3250, 300);
  EfviSimpleOrder sell1(false, 3251, 200);
  EfviSimpleOrder sell2(false, 3252, 200);
  EfviSimpleOrder sell3(false, 3253, 400);
  EfviSimpleOrder sell4(false, 3254, 300);
  EfviSimpleOrder sell5(false, 3255, 100);
  EfviSimpleOrder sell6(false, 3255, 100);

  EfviBboCbListener listener;
  TypedDepthOrderBook order_book;
  order_book.set_bbo_listener(&listener);
  // Add buy orders, should be accepted
  order_book.add(&buy0);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  order_book.add(&buy1);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
  order_book.add(&buy2);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
  order_book.add(&buy3);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
  order_book.add(&buy4);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();

  // Add buy orders past end, should be accepted, but not affect depth
  order_book.add(&buy5);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
  order_book.add(&buy6);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();

  // Add sell orders, should be accepted efviAnd affect bbo
  order_book.add(&sell2);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  order_book.add(&sell1);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  order_book.add(&sell0);
  BOOST_CHECK_EQUAL(1, listener.changes_.size());
  listener.reset();
  // Add sell orders worse than best bid, should not effect bbo
  order_book.add(&sell5);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
  order_book.add(&sell4);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
  order_book.add(&sell3);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();

  // Add sell order past end, should be accepted, but not affect depth
  order_book.add(&sell6);
  BOOST_CHECK_EQUAL(0, listener.changes_.size());
  listener.reset();
}

BOOST_AUTO_TEST_CASE(TestTradeCallbacks) 
{
  EfviSimpleOrder order0(false, 3250, 100);
  EfviSimpleOrder order1(true,  3250, 800);
  EfviSimpleOrder order2(false, 3230, 0);
  EfviSimpleOrder order3(false, 3240, 200);
  EfviSimpleOrder order4(true,  3250, 600);

  EfviTradeCbListener listener;
  EfviTypedOrderBook order_book;
  order_book.set_trade_listener(&listener);
  // Add order, should be accepted
  order_book.add(&order0);
  BOOST_CHECK_EQUAL(0, listener.quantities_.size());
  listener.reset();
  // Add matching order, should result in a trade
  order_book.add(&order1);
  BOOST_CHECK_EQUAL(1, listener.quantities_.size());
  BOOST_CHECK_EQUAL(1, listener.costs_.size());
  BOOST_CHECK_EQUAL(100, listener.quantities_[0]);
  BOOST_CHECK_EQUAL(100 * 3250, listener.costs_[0]);
  listener.reset();
  // Add invalid order, should be rejected
  order_book.add(&order2);
  BOOST_CHECK_EQUAL(0, listener.quantities_.size());
  listener.reset();
  // Cancel only valid order, should be cancelled
  order_book.cancel(&order1);
  BOOST_CHECK_EQUAL(0, listener.quantities_.size());
  listener.reset();
  // Cancel filled order, should be rejected
  order_book.cancel(&order0);
  BOOST_CHECK_EQUAL(0, listener.quantities_.size());
  listener.reset();
  // Add a new order efviAnd replace it, should be replaced
  order_book.add(&order3);
  order_book.replace(&order3, 0, 3250);
  BOOST_CHECK_EQUAL(0, listener.quantities_.size());
  listener.reset();
  // Add matching order, should be accepted, followed by a fill
  order_book.add(&order4);
  BOOST_CHECK_EQUAL(1, listener.quantities_.size());
  BOOST_CHECK_EQUAL(1, listener.costs_.size());
  listener.reset();
  // Replace matched order, with too large of a size decrease, replace
  // should be rejected
  order_book.replace(&order3, -500);
  BOOST_CHECK_EQUAL(0, listener.quantities_.size());
}

} // namespace liquibook


