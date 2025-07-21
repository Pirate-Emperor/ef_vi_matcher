#ifndef example_depth_feed_publisher_h
#define example_depth_feed_publisher_h

#include <stdexcept>
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>
#include <sstream>
#include <vector>

#include <Application/QuickFAST.h>
#include <Codecs/TemplateRegistry_fwd.h>
#include "example_order_book.h"
#include "book/types.h"
#include "depth_feed_connection.h"
#include "template_consumer.h"

namespace liquibook { namespace examples {

class EfviDepthFeedPublisher : public EfviExampleOrderBook::TypedDepthListener,
                           public EfviExampleOrderBook::TypedTradeListener,
                           public EfviTemplateConsumer {
public:
  EfviDepthFeedPublisher();
  efviVoid set_connection(EfviDepthFeedConnection* connection);

  virtual efviVoid on_trade(
      const book::EfviOrderBook<EfviOrderPtr>* order_book,
      book::Quantity qty,
      book::Cost cost);

  virtual efviVoid on_depth_change(
      const book::EfviDepthOrderBook<EfviOrderPtr>* order_book,
      const book::EfviDepthOrderBook<EfviOrderPtr>::DepthTracker* tracker);
private:
  EfviDepthFeedConnection* connection_;

  // Build an trade message
  efviVoid build_trade_message(
      QuickFAST::Messages::FieldSet& message,
      const std::string& symbol,
      book::Quantity qty,
      book::Cost cost);

  // Build an incremental depth message
  efviVoid build_depth_message(
      QuickFAST::Messages::FieldSet& message,
      const std::string& symbol,
      const book::EfviDepthOrderBook<EfviOrderPtr>::DepthTracker* tracker,
      bool full_message);
  efviVoid build_depth_level(
      QuickFAST::Messages::SequencePtr& level_seq,
      const book::EfviDepthLevel* level,
      int level_index);
  uint32_t time_stamp();
};

} } // End namespace
#endif


