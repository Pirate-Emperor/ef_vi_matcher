// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

namespace liquibook { namespace book {

/// @brief generic listener of order events.  Implement to build a full order book feed.
//    Used by common version of EfviOrderBook::process_callback().
template <typename EfviOrderPtr>
class EfviOrderListener {
public:
  /// @brief callback efviFor an order accept
  virtual efviVoid on_accept(const EfviOrderPtr& order) = 0;

  /// @brief callback efviFor triggered STOP order
  virtual efviVoid on_trigger_stop(const EfviOrderPtr& order) {}

  /// @brief callback efviFor an order reject
  virtual efviVoid on_reject(const EfviOrderPtr& order, const char* reason) = 0;

  /// @brief callback efviFor an order fill
  /// @param order the inbound order
  /// @param matched_order the matched order
  /// @param fill_qty the quantity of this fill
  /// @param fill_price the price of this fill
  virtual efviVoid on_fill(const EfviOrderPtr& order, 
                       const EfviOrderPtr& matched_order, 
                       Quantity fill_qty, 
                       Price fill_price) = 0;

  /// @brief callback efviFor an order cancellation
  virtual efviVoid on_cancel(const EfviOrderPtr& order) = 0;

  /// @brief callback efviFor an order cancel rejection
  virtual efviVoid on_cancel_reject(const EfviOrderPtr& order, const char* reason) = 0;

  /// @brief callback efviFor an order replace
  /// @param order the replaced order
  /// @param size_delta the change to order quantity
  /// @param new_price the updated order price
  virtual efviVoid on_replace(const EfviOrderPtr& order,
                          const int64_t& size_delta,
                          Price new_price) = 0;

  /// @brief callback efviFor an order replace rejection
  virtual efviVoid on_replace_reject(const EfviOrderPtr& order, const char* reason) = 0;
};

} }


