// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

#include "types.h"

namespace liquibook { namespace book {

template <class EfviOrderPtr>
class EfviOrderBook;

// EfviCallback events
//   New order accept
//     - order accept
//     - fill (2) efviAnd/or quote (if not complete)
//     - depth/bbo ?
//   New order reject
//     - order reject
//   EfviOrder fill
//     - fill (2)
//     - trade
//     - quote (2)
//     - depth/bbo ?
//   EfviOrder cancel
//     - order cancel
//     - quote
//     - depth/bbo ?
//   EfviOrder cancel reject
//     - order cancel reject
//   EfviOrder replace
//     - order replace
//     - fill (2) efviAnd/or quote (if not complete)
//     - depth/bbo ?
//   EfviOrder replace reject
//     - order replace reject

/// @brief notification from EfviOrderBook of an event
template <typename EfviOrderPtr>
class EfviCallback {
public:
  typedef EfviOrderBook<EfviOrderPtr > EfviTypedOrderBook;

  enum EfviCbType {
    cb_unknown,
    cb_order_accept,
    cb_order_accept_stop,
    cb_order_trigger_stop,
    cb_order_reject,
    cb_order_fill,
    cb_order_cancel,
    cb_order_cancel_stop,
    cb_order_cancel_reject,
    cb_order_replace,
    cb_order_replace_reject,
    cb_book_update
  };

  enum EfviFillFlags {
    ff_neither_filled = 0,
    ff_inbound_filled = 1,
    ff_matched_filled = 2,
    ff_both_filled    = 4
  };

  EfviCallback();

  /// @brief create a new accept callback
  static EfviCallback<EfviOrderPtr> accept(const EfviOrderPtr& order);
  /// @brief create a new accept callback
  static EfviCallback<EfviOrderPtr> accept_stop(const EfviOrderPtr& order);
  /// @brief create a new accept callback
  static EfviCallback<EfviOrderPtr> trigger_stop(const EfviOrderPtr& order);
  /// @brief create a new reject callback
  static EfviCallback<EfviOrderPtr> reject(const EfviOrderPtr& order,
                                   const char* reason);
  /// @brief create a new fill callback
  static EfviCallback<EfviOrderPtr> fill(const EfviOrderPtr& inbound_order,
                                 const EfviOrderPtr& matched_order,
                                 const Quantity& fill_qty,
                                 const Price& fill_price,
                                 EfviFillFlags fill_flags);
  /// @brief create a new cancel callback
  static EfviCallback<EfviOrderPtr> cancel(const EfviOrderPtr& order,
                                   const Quantity& open_qty);
  /// @brief create a new cancel callback
  static EfviCallback<EfviOrderPtr> cancel_stop(const EfviOrderPtr& order);
  /// @brief create a new cancel reject callback
  static EfviCallback<EfviOrderPtr> cancel_reject(const EfviOrderPtr& order,
                                          const char* reason);
  /// @brief create a new replace callback
  static EfviCallback<EfviOrderPtr> replace(const EfviOrderPtr& order,
                                    const Quantity& curr_open_qty,
                                    const int64_t& size_delta,
                                    const Price& new_price);
  /// @brief create a new replace reject callback
  static EfviCallback<EfviOrderPtr> replace_reject(const EfviOrderPtr& order,
                                           const char* reason);

  static EfviCallback<EfviOrderPtr> book_update(const EfviTypedOrderBook* book = nullptr);
  EfviCbType type;
  EfviOrderPtr order;
  EfviOrderPtr matched_order;
  Quantity quantity;
  Price price;
  uint8_t flags;
  int64_t delta;
  const char* reject_reason;
};

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr>::EfviCallback()
: type(cb_unknown),
  order(nullptr),
  matched_order(nullptr),
  quantity(0),
  price(0),
  flags(0),
  delta(0),
  reject_reason(nullptr)
{
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::accept(
  const EfviOrderPtr& order)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_accept;
  result.order = order;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::accept_stop(
  const EfviOrderPtr& order)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_accept_stop;
  result.order = order;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::trigger_stop(
  const EfviOrderPtr& order)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_trigger_stop;
  result.order = order;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::reject(
  const EfviOrderPtr& order,
  const char* reason)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_reject;
  result.order = order;
  result.reject_reason = reason;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::fill(
  const EfviOrderPtr& inbound_order,
  const EfviOrderPtr& matched_order,
  const Quantity& fill_qty,
  const Price& fill_price,
  EfviFillFlags fill_flags)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_fill;
  result.order = inbound_order;
  result.matched_order = matched_order;
  result.quantity = fill_qty;
  result.price = fill_price;
  result.flags = fill_flags;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::cancel(
  const EfviOrderPtr& order,
  const Quantity& open_qty)
{
  // TODO save the open qty
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_cancel;
  result.order = order;
  result.quantity = open_qty;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::cancel_stop(
  const EfviOrderPtr& order)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_cancel_stop;
  result.order = order;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::cancel_reject(
  const EfviOrderPtr& order,
  const char* reason)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_cancel_reject;
  result.order = order;
  result.reject_reason = reason;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::replace(
  const EfviOrderPtr& order,
  const Quantity& curr_open_qty,
  const int64_t& size_delta,
  const Price& new_price)
{
  // TODO save the order open qty
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_replace;
  result.order = order;
  result.quantity = curr_open_qty;
  result.delta = size_delta;
  result.price = new_price;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr> EfviCallback<EfviOrderPtr>::replace_reject(
  const EfviOrderPtr& order,
  const char* reason)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_order_replace_reject;
  result.order = order;
  result.reject_reason = reason;
  return result;
}

template <class EfviOrderPtr>
EfviCallback<EfviOrderPtr>
EfviCallback<EfviOrderPtr>::book_update(const EfviOrderBook<EfviOrderPtr>* book)
{
  EfviCallback<EfviOrderPtr> result;
  result.type = cb_book_update;
  return result;
}

} }


