#include "example_order_book.h"

namespace liquibook { namespace examples {

EfviExampleOrderBook::EfviExampleOrderBook(const std::string& symbol)
: symbol_(symbol)
{
}

const std::string&
EfviExampleOrderBook::symbol() const
{
  return symbol_;
}

} } // End namespace



