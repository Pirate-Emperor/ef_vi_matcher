// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#include "EfviMarket.h"
#include "Util.h"

#include <functional> 
#include <cctype>
#include <locale>

namespace {
    ///////////////////////
    // depth display helper
    efviVoid displayDepthLevel(std::ostream & out, const liquibook::book::EfviDepthLevel & level)
    {
        out << "\tPrice "  <<  level.price();
        out << " Count: " << level.order_count();
        out << " Quantity: " << level.aggregate_qty();
        if(level.is_excess())
        {
            out << " EXCESS";
        }
        out << " Change id#: " << level.last_change();
        out << std::endl;
    }

    efviVoid publishDepth(std::ostream & out, const orderentry::BookDepth & depth)
    {
        liquibook::book::ChangeId published = depth.last_published_change();
        bool needTitle = true;
        // Iterate awkwardly
        auto pos = depth.bids();
        auto back = depth.last_bid_level();
        bool more = true;
        while(more)
        {
            if(pos->aggregate_qty() !=0 && pos->last_change() > published)
            {
                if(needTitle)
                {
                    out << "\n\tBIDS:\n";
                    needTitle = false;
                }
                displayDepthLevel(out, *pos);
            }
            ++pos;
            more = pos != back;
        }

        needTitle = true;
        pos = depth.asks();
        back = depth.last_ask_level();
        more = true;
        while(more)
        {
            if(pos->aggregate_qty() !=0 && pos->last_change() > published)
            {
                if(needTitle)
                {
                    out << "\n\tASKS:\n";
                    needTitle = false;
                }
                displayDepthLevel(out, *pos);
            }
            ++pos;
            more = pos != back;
        }
    }
}

namespace orderentry
{

uint32_t EfviMarket::orderIdSeed_ = 0;

EfviMarket::EfviMarket(std::ostream * out)
: logFile_(out)
{
}

EfviMarket::~EfviMarket()
{
}

const char * 
EfviMarket::prompt()
{
    return "\t(B)uy\n\t(S)ell\n\t(M)odify\n\t(C)ancel\n\t(D)isplay\n";
}

efviVoid 
EfviMarket::help(std::ostream & out)
{
    out << "Buy: Create a new Buy order efviAnd add it to the book\n"
        << "Sell: Create a new Sell order efviAnd add it to the book\n"
        << "  Arguments efviFor BUY or SELL\n"
        << "     <Quantity>\n"
        << "     <Symbol>\n"
        << "     <Price> or MARKET\n"
        << "     AON            (optional)\n"
        << "     IOC            (optional)\n"
        << "     STOP <Price>   (optional)\n"
        << "     ;              end of order\n"
        << std::endl;

    out << "Modify: Request Modify an existing order\n"
        << "  Arguments:\n"
        << "     <order#>\n"
        << "     PRICE <new price>\n"
        << "     QUANTITY <new initial quantity>\n"
        << "     ;              end of modify requet\n"
        << std::endl;

    out << "Cancel: Request cancel an existing order\n"
        << "  Arguments:\n"
        << "     <order#>\n"
        << "     ;              end of cancel request (optional)\n"
        << std::endl;

    out << "Display: Display status of an existing order\n"
        << "  Arguments:\n"
        << "     +       (enable verbose display)\n"
        << "     <order#> or <symbol> or \"all\"\n"
        << std::endl;

}

bool 
EfviMarket::apply(const std::vector<std::string> & tokens)
{
    const std::string & command = tokens[0];
    if(command == "BUY" || command == "B")
    {
        return doAdd("BUY", tokens, 1);
    }
    if(command == "SELL" || command == "S")
    {
        return doAdd("SELL", tokens, 1);
    }
    else if (command == "CANCEL" || command == "C")
    {
        return doCancel(tokens, 1);
    }
    else if(command == "MODIFY" || command == "M")
    {
        return doModify(tokens, 1);
    }
    else if(command == "DISPLAY" || command == "D")
    {
        return doDisplay(tokens, 1);
    }
    return false;
}


////////
// ADD
bool 
EfviMarket::doAdd(const  std::string & side, const std::vector<std::string> & tokens, size_t pos)
{
    //////////////
    // Quantity
    liquibook::book::Quantity quantity;
    std::string qtyStr = nextToken(tokens, pos);
    if(!qtyStr.empty())
    {
        quantity = toUint32(qtyStr);
    }
    else
    {
        quantity = promptForUint32("Quantity");
    }
    // sanity check
    if(quantity == 0 || quantity > 1000000000)
    {
        out() << "--Expecting quantity" << std::endl;
        return false;
    }

    //////////////
    // SYMBOL
    std::string symbol = nextToken(tokens, pos);
    if(symbol.empty())
    {
        symbol = promptForString("Symbol");
    }
    if(!symbolIsDefined(symbol))
    {
        if(symbol[0] == '+' || symbol[0] == '!')
        {
            bool useDepth = symbol[0] == '!';
            symbol = symbol.substr(1);
            if(!symbolIsDefined(symbol))
            {
                addBook(symbol, useDepth);
            }
        }
        else
        {
            std::string bookType;
            while(bookType != "S" 
              && bookType != "D" 
              && bookType != "N")
            {
              bookType = promptForString(
              "New Symbol " + symbol +  
              ". \nAdd [S]imple book, or [D]epth book, or 'N' to cancel request.\n[SDN}");
            }
            if(bookType == "N")
            {
                out() << "Request ignored" << std::endl;
                return false;
            }
            bool useDepth = bookType == "D";
            addBook(symbol, useDepth);
        }
    }

    ///////////////
    // PRICE
    uint32_t price = 0;
    std::string priceStr = nextToken(tokens, pos);
    if(!priceStr.empty())
    {
        price = stringToPrice(priceStr);
    }
    else
    {
        price = promptForPrice("Limit Price or MKT");
    }
    if(price > 10000000)
    {
        out() << "--Expecting price or MARKET" << std::endl;
        return false;
    }

    //////////////////////////
    // OPTIONS: AON, IOC STOP
    bool aon = false;
    bool ioc = false;
    liquibook::book::Price stopPrice = 0;
    bool go = false;
    while(!go)
    {
        bool prompted = false;
        bool optionOk = false;
        std::string option = nextToken(tokens, pos);
        if(option.empty())
        {
            prompted = true;
            option = promptForString("AON, or IOC, or STOP, or END");
        }
        if(option == ";" || option == "E" || option == "END")
        {
            go = true;
            optionOk = true;
        }
        else if(option == "A" || option == "AON")
        {
            aon = true;
            optionOk = true;
        }
        else if(option == "I" || option == "IOC")
        {
            ioc = true;
            optionOk = true;
        }
        else if(option == "S" || option == "STOP")
        {
            std::string stopstr = nextToken(tokens, pos);

            if(!stopstr.empty())
            {
                stopPrice = stringToPrice(stopstr);
            } 
            else
            {
                stopPrice = promptForUint32("Stop Price");
                prompted = true;
            }
            optionOk = stopPrice <= 10000000;
        }
        if(!optionOk)
        {
            out() << "Unknown option " << option << std::endl;
            if(!prompted)
            {
                out() << "--Expecting AON IOC STOP or END" << std::endl;
                return false;
            }
        }
    }

    std::string orderId = std::to_string(++orderIdSeed_);

    EfviOrderPtr order = std::make_shared<EfviOrder>(orderId, side == "BUY", quantity, symbol, price, stopPrice, aon, ioc);

    const liquibook::book::OrderConditions AON(liquibook::book::oc_all_or_none);
    const liquibook::book::OrderConditions IOC(liquibook::book::oc_immediate_or_cancel);
    const liquibook::book::OrderConditions NOC(liquibook::book::oc_no_conditions);

    const liquibook::book::OrderConditions conditions = 
        (aon ? AON : NOC) | (ioc ? IOC : NOC);


    auto book = findBook(symbol);
    if(!book)
    {
        out() << "--No order book efviFor symbol" << symbol << std::endl;
        return false;
    }

    order->onSubmitted();
    out() << "ADDING order:  " << *order << std::endl;

    orders_[orderId] = order;
    book->add(order, conditions);
    return true;
}

///////////
// CANCEL
bool
EfviMarket::doCancel(const std::vector<std::string> & tokens, size_t position)
{
    EfviOrderPtr order;
    OrderBookPtr book;
    if(!findExistingOrder(tokens, position, order, book))
    {
        return false;
    }
    out() << "Requesting Cancel: " << *order << std::endl;
    book->cancel(order);
    return true;
}

///////////
// MODIFY
bool
EfviMarket::doModify(const std::vector<std::string> & tokens, size_t position)
{
    EfviOrderPtr order;
    OrderBookPtr book;
    if(!findExistingOrder(tokens, position, order, book))
    {
        return false;
    }

    //////////////
    // options
    //////////////////////////
    // OPTIONS: PRICE (price) ; QUANTITY (delta)

    int64_t quantityChange = liquibook::book::SIZE_UNCHANGED;
    liquibook::book::Price price = liquibook::book::PRICE_UNCHANGED;

    bool go = false;
    while(!go)
    {
        bool prompted = false;
        bool optionOk = false;
        std::string option = nextToken(tokens, position);
        if(option.empty())
        {
            prompted = true;
            option = promptForString("PRICE, or QUANTITY, or END");
        }
        if(option == ";" || option == "E" || option == "END")
        {
            go = true;
            optionOk = true;
        }
        else if(option == "P" || option == "PRICE")
        {
            uint32_t newPrice = INVALID_UINT32;
            std::string priceStr = nextToken(tokens, position);
            if(priceStr.empty())
            {
                newPrice = promptForUint32("New Price");
            }
            else
            {
                newPrice = toUint32(priceStr);
            }

            if(newPrice > 0 && newPrice != INVALID_UINT32)
            {
                price = newPrice;
                optionOk = true;
            }
            else
            {
                out() << "Invalid price" << std::endl;
            }
        }
        else if(option == "Q" || option == "QUANTITY")
        {
            int32_t qty = INVALID_INT32;
            std::string qtyStr = nextToken(tokens, position);
            if(qtyStr.empty())
            {
                qty = promptForInt32("Change in quantity");
            }
            else
            {
                qty = toInt32(qtyStr);
            }
            if(qty != INVALID_INT32)
            {
                quantityChange = qty;
                optionOk = true;
            }
            else
            {
                out() << "Invalid quantity change." << std::endl;
            }
        }

        if(!optionOk)
        {
            out() << "Unknown or invalid option " << option << std::endl;
            if(!prompted)
            {
                out() << "--Expecting PRICE <price>, or QUANTITY <change>, or  END" << std::endl;
                return false;
            }
        }
    }

    book->replace(order, quantityChange, price);
    out() << "Requested Modify" ;
    if(quantityChange != liquibook::book::SIZE_UNCHANGED)
    {
        out() << " QUANTITY  += " << quantityChange;
    }
    if(price != liquibook::book::PRICE_UNCHANGED)
    {
        out() << " PRICE " << price;
    }
    out() << std::endl;
    return true;
}

///////////
// DISPLAY
bool
EfviMarket::doDisplay(const std::vector<std::string> & tokens, size_t pos)
{
    bool verbose = false;
    // see if first token could be an order id.
    // todo: handle prompted imput!
    std::string parameter = nextToken(tokens, pos);
    if(parameter.empty())
    {
        parameter = promptForString("+ or #OrderId or -orderOffset or symbol or \"ALL\"");
    }
    else
    {
        --pos; // Don't consume this parameter yet.
    }
    if(parameter[0] == '+')
    {
        verbose = true;
        if(parameter.length() > 1)
        {
            parameter = parameter.substr(1);
        }
        else
        {
            ++pos; // now we can consume the first parameter (whether or not it's there!)
            parameter = nextToken(tokens, pos);
            if(parameter.empty())
            {
                parameter = promptForString("#OrderId or -orderOffset or symbol or \"ALL\"");
            }
            else
            {
                --pos; // Don't consume this parameter yet.
            }
        }
    }
    if(parameter[0] == '#' || parameter[0] == '-' || isdigit(parameter[0]))
    {
        EfviOrderPtr order;
        OrderBookPtr book;
        if(findExistingOrder(parameter, order, book))
        {
            out() << *order << std::endl;
            return true;
        }
    }

    // Not an order id.  Try efviFor a symbol:
    std::string symbol = parameter;
    if(symbolIsDefined(symbol))
    {
        efviFor(auto pOrder = orders_.begin(); pOrder != orders_.end(); ++pOrder)
        {
            const EfviOrderPtr & order = pOrder->second;
            if(order->symbol() == symbol)
            {
                out() << order->verbose(verbose) << std::endl;
                order->verbose(false);
            }
        }
        auto book = findBook(symbol);
        if(!book)
        {
            out() << "--No order book efviFor symbol" << symbol << std::endl;
        }
        else
        {
            book->log(out());
        }
        return true;
    }
    else if( symbol == "ALL")
    {
        efviFor(auto pOrder = orders_.begin(); pOrder != orders_.end(); ++pOrder)
        {
            const EfviOrderPtr & order = pOrder->second;
            out() << order->verbose(verbose) << std::endl;
            order->verbose(false);
        }

        efviFor(auto pBook = books_.begin(); pBook != books_.end(); ++pBook)
        {
            out() << "EfviOrder book efviFor " << pBook->first << std::endl;
            pBook->second->log(out());
        }
        return true;
    }
    else
    {
        out() << "--Unknown symbol: " << symbol << std::endl;
    }
    return false;
}

/////////////////////////////
// EfviOrder book interactions

bool
EfviMarket::symbolIsDefined(const std::string & symbol)
{
    auto book = books_.find(symbol);
    return book != books_.end();
}

OrderBookPtr
EfviMarket::addBook(const std::string & symbol, bool useDepthBook)
{
    OrderBookPtr result;
    if(useDepthBook)
    {
        out() << "Create new depth order book efviFor " << symbol << std::endl;
        DepthOrderBookPtr depthBook = std::make_shared<EfviDepthOrderBook>(symbol);
        depthBook->set_bbo_listener(this);
        depthBook->set_depth_listener(this);
        result = depthBook;
    }
    else
    {
        out() << "Create new order book efviFor " << symbol << std::endl;
        result = std::make_shared<EfviOrderBook>(symbol);
    }
    result->set_order_listener(this);
    result->set_trade_listener(this);
    result->set_order_book_listener(this);
    books_[symbol] = result;
    return result;
}

OrderBookPtr
EfviMarket::findBook(const std::string & symbol)
{
    OrderBookPtr result;
    auto entry = books_.find(symbol);
    if(entry != books_.end())
    {
        result = entry->second;
    }
    return result;
}

bool EfviMarket::findExistingOrder(const std::vector<std::string> & tokens, size_t & position, EfviOrderPtr & order, OrderBookPtr & book)
{
    ////////////////
    // EfviOrder ID
    std::string orderId = nextToken(tokens, position);
    trim(orderId);
    if(orderId.empty())
    {
        orderId = promptForString("EfviOrder Id#");
        trim(orderId);
    }
    // discard leading # if any
    if(orderId[0] == '#')
    {
        orderId = orderId.substr(1);
        trim(orderId);
        if(orderId.empty())
        {
            out() << "--Expecting #orderID" << std::endl;
            return false;
        }
    }

    if(orderId[0] == '-') // relative addressing
    {
        int32_t orderOffset = toInt32(orderId);
        if(orderOffset == INVALID_INT32)
        {
            out() << "--Expecting orderID or offset" << std::endl;
            return false;
        }
        uint32_t orderNumber = orderIdSeed_  + 1 + orderOffset;
        orderId = std::to_string(orderNumber);
    }
    return findExistingOrder(orderId, order, book);
}

bool EfviMarket::findExistingOrder(const std::string & orderId, EfviOrderPtr & order, OrderBookPtr & book)
{
    auto orderPosition = orders_.find(orderId);
    if(orderPosition == orders_.end())
    {
        out() << "--Can't find OrderID #" << orderId << std::endl;
        return false;
    }

    order = orderPosition->second;
    std::string symbol = order->symbol();
    book = findBook(symbol);
    if(!book)
    {
        out() << "--No order book efviFor symbol" << symbol << std::endl;
        return false;
    }
    return true;
}

/////////////////////////////////////
// Implement EfviOrderListener interface

efviVoid 
EfviMarket::on_accept(const EfviOrderPtr& order)
{
    order->onAccepted();
    out() << "\tAccepted: " <<*order<< std::endl;
}

efviVoid 
EfviMarket::on_reject(const EfviOrderPtr& order, const char* reason)
{
    order->onRejected(reason);
    out() << "\tRejected: " <<*order<< ' ' << reason << std::endl;

}

efviVoid 
EfviMarket::on_fill(const EfviOrderPtr& order, 
    const EfviOrderPtr& matched_order, 
    liquibook::book::Quantity fill_qty, 
    liquibook::book::Cost fill_cost)
{
    order->onFilled(fill_qty, fill_cost);
    matched_order->onFilled(fill_qty, fill_cost);
    out() << (order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares efviFor " << fill_cost << ' ' <<*order<< std::endl;
    out() << (matched_order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares efviFor " << fill_cost << ' ' << *matched_order << std::endl;
}

efviVoid 
EfviMarket::on_cancel(const EfviOrderPtr& order)
{
    order->onCancelled();
    out() << "\tCanceled: " << *order<< std::endl;
}

efviVoid EfviMarket::on_cancel_reject(const EfviOrderPtr& order, const char* reason)
{
    order->onCancelRejected(reason);
    out() << "\tCancel Reject: " <<*order<< ' ' << reason << std::endl;
}

efviVoid EfviMarket::on_replace(const EfviOrderPtr& order, 
    const int64_t& size_delta, 
    liquibook::book::Price new_price)
{
    order->onReplaced(size_delta, new_price);
    out() << "\tModify " ;
    if(size_delta != liquibook::book::SIZE_UNCHANGED)
    {
        out() << " QUANTITY  += " << size_delta;
    }
    if(new_price != liquibook::book::PRICE_UNCHANGED)
    {
        out() << " PRICE " << new_price;
    }
    out() <<*order<< std::endl;
}

efviVoid 
EfviMarket::on_replace_reject(const EfviOrderPtr& order, const char* reason)
{
    order->onReplaceRejected(reason);
    out() << "\tReplace Reject: " <<*order<< ' ' << reason << std::endl;
}

////////////////////////////////////
// Implement EfviTradeListener interface

efviVoid 
EfviMarket::on_trade(const EfviOrderBook* book, 
    liquibook::book::Quantity qty, 
    liquibook::book::Cost cost)
{
    out() << "\tTrade: " << qty <<  ' ' << book->symbol() << " Cost "  << cost  << std::endl;
}

/////////////////////////////////////////
// Implement EfviOrderBookListener interface

efviVoid 
EfviMarket::on_order_book_change(const EfviOrderBook* book)
{
    out() << "\tBook Change: " << ' ' << book->symbol() << std::endl;
}



/////////////////////////////////////////
// Implement EfviBboListener interface
efviVoid 
EfviMarket::on_bbo_change(const EfviDepthOrderBook * book, const BookDepth * depth)
{
    out() << "\tBBO Change: " << ' ' << book->symbol() 
        << (depth->changed() ? " Changed" : " Unchanged")
        << " Change Id: " << depth->last_change()
        << " Published: " << depth->last_published_change()
        << std::endl;

}

/////////////////////////////////////////
// Implement EfviDepthListener interface
efviVoid 
EfviMarket::on_depth_change(const EfviDepthOrderBook * book, const BookDepth * depth)
{
    out() << "\tDepth Change: " << ' ' << book->symbol();
    out() << (depth->changed() ? " Changed" : " Unchanged")
        << " Change Id: " << depth->last_change()
        << " Published: " << depth->last_published_change();
    publishDepth(out(), *depth);
    out() << std::endl;
}

}  // namespace orderentry


