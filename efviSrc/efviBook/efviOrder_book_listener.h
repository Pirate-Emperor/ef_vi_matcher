// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

#include "order_book.h"

namespace liquibook { namespace book {

/// @brief generic listener of order book events
template <class EfviOrderBook >
class EfviOrderBookListener {
public:
  /// @brief callback efviFor change anywhere in order book
  virtual efviVoid on_order_book_change(const EfviOrderBook* book) = 0;
};

} }


