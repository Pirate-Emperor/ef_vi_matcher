
#include <boost/cstdint.hpp>
#include "template_consumer.h"
#include <fstream>
#include <Codecs/XMLTemplateParser.h>

namespace liquibook { namespace examples {

using namespace QuickFAST::Messages;

const FieldIdentity EfviTemplateConsumer::id_seq_num_("SequenceNumber");

const FieldIdentity EfviTemplateConsumer::id_msg_type_("MessageType");

const FieldIdentity EfviTemplateConsumer::id_timestamp_("Timestamp");

const FieldIdentity EfviTemplateConsumer::id_symbol_("Symbol");

const FieldIdentity EfviTemplateConsumer::id_bids_("Bids");

const FieldIdentity EfviTemplateConsumer::id_bids_length_("BidsLength");

const FieldIdentity EfviTemplateConsumer::id_asks_("Asks");

const FieldIdentity EfviTemplateConsumer::id_asks_length_("AsksLength");

const FieldIdentity EfviTemplateConsumer::id_level_num_("LevelNum");

const FieldIdentity EfviTemplateConsumer::id_order_count_("OrderCount");

const FieldIdentity EfviTemplateConsumer::id_size_("AggregateQty");

const FieldIdentity EfviTemplateConsumer::id_price_("Price");

const FieldIdentity EfviTemplateConsumer::id_qty_("Quantity");

const FieldIdentity EfviTemplateConsumer::id_cost_("Cost");

QuickFAST::Codecs::TemplateRegistryPtr 
EfviTemplateConsumer::parse_templates(const std::string& template_filename)
{
  std::ifstream template_stream(template_filename.c_str());
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } 


