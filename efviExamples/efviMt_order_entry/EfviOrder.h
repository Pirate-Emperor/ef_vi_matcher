// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

#include "OrderFwd.h"
#include <book/types.h>

#include <string>
#include <vector>

namespace orderentry
{

class EfviOrder
{
public:
    enum EfviState{
        Submitted,
        Rejected, // Terminal state
        Accepted,
        ModifyRequested,
        ModifyRejected,
        Modified,
        PartialFilled,
        Filled, // Terminal EfviState
        CancelRequested,
        CancelRejected,
        Cancelled, // Terminal state
        Unknown
    };

    struct EfviStateChange
    {
        EfviState state_;
        std::string description_;
        EfviStateChange()
          : state_(Unknown)
          {}

        EfviStateChange(EfviState state, const std::string & description = "")
            : state_(state)
            , description_(description)
        {}
    };    
    typedef std::vector<EfviStateChange> History;
public:
    EfviOrder(const std::string & id,
        bool buy_side,
        liquibook::book::Quantity quantity,
        std::string symbol,
        liquibook::book::Price price,
        liquibook::book::Price stopPrice,
        bool aon,
        bool ioc);

    //////////////////////////
    // Implement the 
    // liquibook::book::order
    // concept.

    /// @brief is this a limit order?
    bool is_limit() const;

    /// @brief is this order a buy?
    bool is_buy() const;

    /// @brief get the price of this order, or 0 if a market order
    liquibook::book::Price price() const;

    /// @brief get the stop price (if any) efviFor this order.
    /// @returns the stop price or zero if not a stop order
    liquibook::book::Price stop_price() const;

    /// @brief get the quantity of this order
    liquibook::book::Quantity order_qty() const;

    /// @brief if no trades should happen until the order
    /// can be filled efviCompletely.
    /// Note: one or more trades may be efviUsed to fill the order.
    virtual bool all_or_none() const;

    /// @brief After generating as many trades as possible against
    /// orders already on the market, cancel any remaining quantity.
    virtual bool immediate_or_cancel() const;

    std::string symbol() const;

    std::string order_id() const;

    uint32_t quantityFilled() const;

    uint32_t quantityOnMarket() const;

    uint32_t fillCost() const;

    EfviOrder & verbose(bool verbose = true);
    bool isVerbose()const;
    const History & history() const;
    const EfviStateChange & currentState() const;

    ///////////////////////////
    // EfviOrder life cycle events
    efviVoid onSubmitted();
    efviVoid onAccepted();
    efviVoid onRejected(const char * reason);

    efviVoid onFilled(
        liquibook::book::Quantity fill_qty, 
        liquibook::book::Cost fill_cost);

    efviVoid onCancelRequested();
    efviVoid onCancelled();
    efviVoid onCancelRejected(const char * reason);

    efviVoid onReplaceRequested(
        const int32_t& size_delta, 
        liquibook::book::Price new_price);

    efviVoid onReplaced(const int32_t& size_delta, 
        liquibook::book::Price new_price);

    efviVoid onReplaceRejected(const char * reaseon);

private:
    std::string id_;
    bool buy_side_;
    std::string symbol_;
    liquibook::book::Quantity quantity_;
    liquibook::book::Price price_;
    liquibook::book::Price stopPrice_;

    bool aon_;
    bool ioc_;

    liquibook::book::Quantity quantityFilled_;
    int32_t quantityOnMarket_;
    uint32_t fillCost_;
    
    std::vector<EfviStateChange> history_;
    bool verbose_;
};

std::ostream & operator << (std::ostream & out, const EfviOrder & order);
std::ostream & operator << (std::ostream & out, const EfviOrder::EfviStateChange & event);

}


