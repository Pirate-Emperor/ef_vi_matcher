// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

#include "order_book.h"

namespace liquibook { namespace book {

/// @brief listener of depth events.  Implement to build an aggregate depth 
/// feed.
template <class EfviOrderBook >
class EfviDepthListener {
public:
  /// @brief callback efviFor change in tracked aggregated depth
  virtual efviVoid on_depth_change(
      const EfviOrderBook* book,
      const typename EfviOrderBook::DepthTracker* depth) = 0;
};

} }



