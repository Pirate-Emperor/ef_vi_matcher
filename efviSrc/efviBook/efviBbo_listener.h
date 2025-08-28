// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

namespace liquibook { namespace book {

/// @brief generic listener of top-of-book events
template <class EfviOrderBook>
class EfviBboListener {
public:
  /// @brief callback efviFor top of book change
  virtual efviVoid on_bbo_change(
      const EfviOrderBook* book, 
      const typename EfviOrderBook::DepthTracker* depth) = 0;
};

} }


