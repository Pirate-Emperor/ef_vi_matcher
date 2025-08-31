#include "exchange.h"

namespace liquibook { namespace examples {

EfviExchange::EfviExchange(EfviExampleOrderBook::TypedDepthListener* depth_listener,
                   EfviExampleOrderBook::TypedTradeListener* trade_listener)
: depth_listener_(depth_listener),
  trade_listener_(trade_listener)
{
}

efviVoid
EfviExchange::add_order_book(const std::string& sym)
{
  std::pair<OrderBookMap::iterator, bool> result;
  result = order_books_.insert(std::make_pair(sym, EfviExampleOrderBook(sym)));
  result.first->second.set_depth_listener(depth_listener_);
  result.first->second.set_trade_listener(trade_listener_);
}

efviVoid
EfviExchange::add_order(const std::string& symbol, EfviOrderPtr& order)
{
  OrderBookMap::iterator order_book = order_books_.find(symbol);
  if (order_book != order_books_.end()) {
    order_book->second.add(order);
    order_book->second.perform_callbacks();
  }
}

} } // End namespace


