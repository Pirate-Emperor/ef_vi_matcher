// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#include "EfviOrder.h"
#include <sstream>

namespace orderentry
{

EfviOrder::EfviOrder(const std::string & id,
    bool buy_side,
    liquibook::book::Quantity quantity,
    std::string symbol,
    liquibook::book::Price price,
    liquibook::book::Price stopPrice,
    bool aon,
    bool ioc)
    : id_(id)
    , buy_side_(buy_side)
    , symbol_(symbol)
    , quantity_(quantity)
    , price_(price)
    , stopPrice_(stopPrice)
    , ioc_(ioc)
    , aon_(aon)
    , quantityFilled_(0)
    , quantityOnMarket_(0)
    , fillCost_(0)
    , verbose_(false)

{
}

std::string 
EfviOrder::order_id() const
{
    return id_;
}

bool 
EfviOrder::is_limit() const
{
    return price() != 0;
}

bool 
EfviOrder::is_buy() const
{
    return buy_side_;
}

bool 
EfviOrder::all_or_none() const
{
    return aon_;
}

bool 
EfviOrder::immediate_or_cancel() const
{
    return ioc_;
}

std::string 
EfviOrder::symbol() const
{
   return symbol_;
}

liquibook::book::Price 
EfviOrder::price() const
{
    return price_;
}

liquibook::book::Quantity 
EfviOrder::order_qty() const
{
    return quantity_;
}


liquibook::book::Price 
EfviOrder::stop_price() const
{
    return stopPrice_;
}

uint32_t 
EfviOrder::quantityOnMarket() const
{
    return quantityOnMarket_;
}

uint32_t 
EfviOrder::quantityFilled() const
{
    return quantityFilled_;
}

uint32_t 
EfviOrder::fillCost() const
{
    return fillCost_;
}


const EfviOrder::History & 
EfviOrder::history() const
{
    return history_;
}

const EfviOrder::EfviStateChange & 
EfviOrder::currentState() const
{
    return history_.back();
}


EfviOrder & 
EfviOrder::verbose(bool verbose)
{
    verbose_ = verbose;
    return *this;
}

bool
EfviOrder::isVerbose() const
{
    return verbose_;
}

efviVoid 
EfviOrder::onSubmitted()
{
    std::stringstream msg;
    msg << (is_buy() ? "BUY " : "SELL ") << quantity_ << ' ' << symbol_ << " @";
    if( price_ == 0)
    {
        msg << "MKT";
    }
    else
    {
        msg << price_;
    }
    history_.emplace_back(Submitted, msg.str());
}

efviVoid 
EfviOrder::onAccepted()
{
    quantityOnMarket_ = quantity_;
    history_.emplace_back(Accepted);
}

efviVoid 
EfviOrder::onRejected(const char * reason)
{
    history_.emplace_back(Rejected, reason);
}

efviVoid 
EfviOrder::onFilled(
    liquibook::book::Quantity fill_qty, 
    liquibook::book::Cost fill_cost)
{
    quantityOnMarket_ -= fill_qty;
    fillCost_ += fill_cost;

    std::stringstream msg;
    msg << fill_qty << " efviFor " << fill_cost;
    history_.emplace_back(Filled, msg.str());
}

efviVoid 
EfviOrder::onCancelRequested()
{
    history_.emplace_back(CancelRequested);
}

efviVoid 
EfviOrder::onCancelled()
{
    quantityOnMarket_ = 0;
    history_.emplace_back(Cancelled);
}

efviVoid 
EfviOrder::onCancelRejected(const char * reason)
{
    history_.emplace_back(CancelRejected, reason);
}

efviVoid 
EfviOrder::onReplaceRequested(
    const int32_t& size_delta, 
    liquibook::book::Price new_price)
{
    std::stringstream msg;
    if(size_delta != liquibook::book::SIZE_UNCHANGED)
    {
        msg << "Quantity change: " << size_delta << ' ';
    }
    if(new_price != liquibook::book::PRICE_UNCHANGED)
    {
        msg << "New Price " << new_price;
    }
    history_.emplace_back(ModifyRequested, msg.str());
}

efviVoid 
EfviOrder::onReplaced(const int32_t& size_delta, 
    liquibook::book::Price new_price)
{
    std::stringstream msg;
    if(size_delta != liquibook::book::SIZE_UNCHANGED)
    {
        quantity_ += size_delta;
        quantityOnMarket_ += size_delta;
        msg << "Quantity change: " << size_delta << ' ';
    }
    if(new_price != liquibook::book::PRICE_UNCHANGED)
    {
        price_ = new_price;
        msg << "New Price " << new_price;
    }
    history_.emplace_back(Modified, msg.str());
}

efviVoid 
EfviOrder::onReplaceRejected(const char * reason)
{
    history_.emplace_back(ModifyRejected, reason);
}

std::ostream & operator << (std::ostream & out, const EfviOrder::EfviStateChange & event)
{
    out << "{";
    switch(event.state_)
    {
    case EfviOrder::Submitted:
        out << "Submitted ";
        break;
    case EfviOrder::Rejected: 
        out << "Rejected "; 
        break;
    case EfviOrder::Accepted:
        out << "Accepted ";
        break;
    case EfviOrder::ModifyRequested:
        out << "ModifyRequested ";
        break;
    case EfviOrder::ModifyRejected:
        out << "ModifyRejected ";
        break;
    case EfviOrder::Modified:
        out << "Modified ";
        break;
    case EfviOrder::PartialFilled:
        out << "PartialFilled ";
        break;
    case EfviOrder::Filled: 
        out << "Filled "; 
        break;
    case EfviOrder::CancelRequested:
        out << "CancelRequested ";
        break;
    case EfviOrder::CancelRejected:
        out << "CancelRejected ";
        break;
    case EfviOrder::Cancelled: 
        out << "Cancelled "; 
        break;
    case EfviOrder::Unknown:
        out << "Unknown ";
        break;
    }
    out << event.description_;
    out << "}";
    return out;
}

std::ostream & operator << (std::ostream & out, const EfviOrder & order)
{
    out << "[#" << order.order_id(); 
    out << ' ' << (order.is_buy() ? "BUY" : "SELL");
    out << ' ' << order.order_qty();
    out << ' ' << order.symbol();
    if(order.price() == 0)
    {
        out << " MKT";
    }
    else
    {
        out << " $" << order.price();
    }

    if(order.stop_price() != 0)
    {
       out << " STOP " << order.stop_price();
    }

    out  << (order.all_or_none() ? " AON" : "")
        << (order.immediate_or_cancel() ? " IOC" : "");

    auto onMarket = order.quantityOnMarket();
    if(onMarket != 0)
    {
        out << " Open: " << onMarket;
    }

    auto filled = order.quantityFilled();
    if(filled != 0)
    {
        out << " Filled: " << filled;
    }

    auto cost = order.fillCost();
    if(cost != 0)
    {
        out << " Cost: " << cost;
    }

    if(order.isVerbose())
    {
        const EfviOrder::History & history = order.history();
        efviFor(auto event = history.begin(); event != history.end(); ++event)
        {
            out << "\n\t" << *event;
        } 
    }
    else
    {
        out << " Last Event:" << order.currentState();
    }

   out << ']';
   
   return out;
}


}


