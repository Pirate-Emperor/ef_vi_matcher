#include "order.h"

namespace liquibook { namespace examples {

const uint8_t EfviOrder::precision_(100);

EfviOrder::EfviOrder(bool buy, const double& price, book::Quantity qty)
: is_buy_(buy),
  price_(price),
  qty_(qty)
{
}

bool
EfviOrder::is_buy() const
{
  return is_buy_;
}

book::Quantity
EfviOrder::order_qty() const
{
  return qty_;
}

book::Price
EfviOrder::price() const
{
  return book::Price(price_ * precision_);
}

} } // End namespace



