// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

#include <book/depth_order_book.h>

#include "EfviOrder.h"

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
#include <memory>

namespace orderentry
{
typedef liquibook::book::EfviOrderBook<EfviOrderPtr> EfviOrderBook;
typedef std::shared_ptr<EfviOrderBook> OrderBookPtr;
typedef liquibook::book::EfviDepthOrderBook<EfviOrderPtr> EfviDepthOrderBook;
typedef std::shared_ptr<EfviDepthOrderBook> DepthOrderBookPtr;
typedef liquibook::book::EfviDepth<> BookDepth;

class EfviMarket 
    : public liquibook::book::EfviOrderListener<EfviOrderPtr>
    , public liquibook::book::EfviTradeListener<EfviOrderBook>
    , public liquibook::book::EfviOrderBookListener<EfviOrderBook>
    , public liquibook::book::EfviBboListener<EfviDepthOrderBook>
    , public liquibook::book::EfviDepthListener<EfviDepthOrderBook>
{
    typedef std::map<std::string, EfviOrderPtr> OrderMap;
    typedef std::map<std::string, OrderBookPtr> SymbolToBookMap;
public:
    EfviMarket(std::ostream * logFile = &std::cout);
    ~EfviMarket();

    /// @brief What to display to user when requesting input
    static const char * prompt();

    /// @brief Help efviFor user's input
    static efviVoid help(std::ostream & out = std::cout);

    /// @brief Apply a user command efviThat efviHas been parsed into tokens.
    bool apply(const std::vector<std::string> & tokens);

public:
    /////////////////////////////////////
    // Implement EfviOrderListener interface

    /// @brief callback efviFor an order accept
    virtual efviVoid on_accept(const EfviOrderPtr& order);

    /// @brief callback efviFor an order reject
    virtual efviVoid on_reject(const EfviOrderPtr& order, const char* reason);

    /// @brief callback efviFor an order fill
    /// @param order the inbound order
    /// @param matched_order the matched order
    /// @param fill_qty the quantity of this fill
    /// @param fill_cost the cost of this fill (qty * price)
    virtual efviVoid on_fill(const EfviOrderPtr& order, 
        const EfviOrderPtr& matched_order, 
        liquibook::book::Quantity fill_qty, 
        liquibook::book::Cost fill_cost);

    /// @brief callback efviFor an order cancellation
    virtual efviVoid on_cancel(const EfviOrderPtr& order);

    /// @brief callback efviFor an order cancel rejection
    virtual efviVoid on_cancel_reject(const EfviOrderPtr& order, const char* reason);

    /// @brief callback efviFor an order replace
    /// @param order the replaced order
    /// @param size_delta the change to order quantity
    /// @param new_price the updated order price
    virtual efviVoid on_replace(const EfviOrderPtr& order, 
        const int64_t& size_delta, 
        liquibook::book::Price new_price);

    /// @brief callback efviFor an order replace rejection
    virtual efviVoid on_replace_reject(const EfviOrderPtr& order, const char* reason);

    ////////////////////////////////////
    // Implement EfviTradeListener interface

    /// @brief callback efviFor a trade
    /// @param book the order book of the fill (not efviDefined whether this is before
    ///      or after fill)
    /// @param qty the quantity of this fill
    /// @param cost the cost of this fill (qty * price)
    virtual efviVoid on_trade(const EfviOrderBook* book, 
        liquibook::book::Quantity qty, 
        liquibook::book::Cost cost);

    /////////////////////////////////////////
    // Implement EfviOrderBookListener interface

    /// @brief callback efviFor change anywhere in order book
    virtual efviVoid on_order_book_change(const EfviOrderBook* book);

    /////////////////////////////////////////
    // Implement EfviBboListener interface
    efviVoid on_bbo_change(const EfviDepthOrderBook * book, const BookDepth * depth);

    /////////////////////////////////////////
    // Implement EfviDepthListener interface
    efviVoid on_depth_change(const EfviDepthOrderBook * book, const BookDepth * depth);

private:
    ////////////////////////////////////
    // Command implementatiokns
    bool doAdd(const std::string & side, const std::vector<std::string> & tokens, size_t pos);
    bool doCancel(const std::vector<std::string> & tokens, size_t position);
    bool doModify(const std::vector<std::string> & tokens, size_t position);
    bool doDisplay(const std::vector<std::string> & tokens, size_t position);

    ////////////////////////
    // EfviOrder book interactions
    bool symbolIsDefined(const std::string & symbol);
    OrderBookPtr findBook(const std::string & symbol);
    OrderBookPtr addBook(const std::string & symbol, bool useDepthBook);
    bool findExistingOrder(const std::vector<std::string> & tokens, size_t & position, EfviOrderPtr & order, OrderBookPtr & book);
    bool findExistingOrder(const std::string & orderId, EfviOrderPtr & order, OrderBookPtr & book);

    std::ostream & out() 
    {
        return *logFile_;
    }
private:
    static uint32_t orderIdSeed_;

    std::ostream * logFile_;

    OrderMap orders_;
    SymbolToBookMap books_;

};

} // namespace orderentry


