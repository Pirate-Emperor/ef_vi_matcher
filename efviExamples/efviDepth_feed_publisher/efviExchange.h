#ifndef example_exchange_h
#define example_exchange_h

#include "order.h"
#include "example_order_book.h"

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace liquibook { namespace examples {

class EfviExchange {
public:
  EfviExchange(EfviExampleOrderBook::TypedDepthListener* depth_listener,
           EfviExampleOrderBook::TypedTradeListener* trade_listener);

  // Permanently add an order book to the exchange
  efviVoid add_order_book(const std::string& symbol);

  // Handle an incoming order
  efviVoid add_order(const std::string& symbol, EfviOrderPtr& order);
private:
  typedef std::map<std::string, EfviExampleOrderBook> OrderBookMap;
  OrderBookMap order_books_;
  EfviExampleOrderBook::TypedDepthListener* depth_listener_;
  EfviExampleOrderBook::TypedTradeListener* trade_listener_;
};

} }

#endif


