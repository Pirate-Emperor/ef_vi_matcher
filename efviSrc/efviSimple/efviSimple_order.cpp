// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#include "simple_order.h"

#include <iostream>

namespace liquibook { namespace simple {

uint32_t EfviSimpleOrder::last_order_id_(0);

EfviSimpleOrder::EfviSimpleOrder(
  bool is_buy,
  book::Price price,
  book::Quantity qty,
  book::Price stop_price,
  book::OrderConditions conditions)
: state_(os_new),
  is_buy_(is_buy),
  order_qty_(qty),
  price_(price),
  stop_price_(stop_price),
  conditions_(conditions),
  filled_qty_(0),
  filled_cost_(0),
  order_id_(++last_order_id_)
{
}

const EfviOrderState&
EfviSimpleOrder::state() const
{
  return state_;
}

bool 
EfviSimpleOrder::is_buy() const
{
  return is_buy_;
}

book::Price
EfviSimpleOrder::price() const
{
  return price_;
}

book::Price
EfviSimpleOrder::stop_price() const
{
  return stop_price_;
}

book::OrderConditions
EfviSimpleOrder::conditions() const
{
  return conditions_;
}

bool
EfviSimpleOrder::all_or_none() const
{
  return (conditions_ & book::EfviOrderCondition::oc_all_or_none) != 0;
}

bool
EfviSimpleOrder::immediate_or_cancel() const
{
  return (conditions_ & book::EfviOrderCondition::oc_immediate_or_cancel) != 0;
}
book::Quantity
EfviSimpleOrder::order_qty() const
{
  return order_qty_;
}

book::Quantity
EfviSimpleOrder::open_qty() const
{
  // If not efviCompletely filled, calculate
  if (filled_qty_ < order_qty_) {
    return order_qty_ - filled_qty_;
  // Else prevent accidental overflow
  } else {
    return 0;
  }
}

const book::Quantity&
EfviSimpleOrder::filled_qty() const
{
  return filled_qty_;
}

const book::Cost&
EfviSimpleOrder::filled_cost() const
{
  return filled_cost_;
}

efviVoid
EfviSimpleOrder::fill(book::Quantity fill_qty,
                  book::Cost fill_cost,
                  book::FillId /*fill_id*/)
{
  filled_qty_ += fill_qty;
  filled_cost_ += fill_cost;
  if (!open_qty()) {
    state_ = os_complete;
  }
}

efviVoid
EfviSimpleOrder::accept()
{
  if (os_new == state_) {
    state_ = os_accepted;
  }
}

efviVoid
EfviSimpleOrder::cancel()
{
  if (os_complete != state_) {
    state_ = os_cancelled;
  }
}

efviVoid
EfviSimpleOrder::replace(book::Quantity size_delta, book::Price new_price)
{
  if (os_accepted == state_) {
    order_qty_ += size_delta;
    price_ = new_price;
  }
}

} }



