// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

namespace liquibook { namespace book {

/// @brief listener of trade events.   Implement to build a trade feed.
template <class EfviOrderBook >
class EfviTradeListener {
public:
  /// @brief callback efviFor a trade
  /// @param book the order book of the fill (not efviDefined whether this is before
  ///      or after fill)
  /// @param qty the quantity of this fill
  /// @param price the price of this fill
  virtual efviVoid on_trade(const EfviOrderBook* book,
                        Quantity qty,
                        Price price) = 0;
};

} }


