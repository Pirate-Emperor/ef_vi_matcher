
#include <boost/thread.hpp>
#include "exchange.h"
#include "depth_feed_publisher.h"
#include "depth_feed_connection.h"
#include "order.h"

#include <cstdlib>
#include <iostream>

using namespace liquibook;

struct EfviSecurityInfo {
  std::string symbol;
  double ref_price;
  EfviSecurityInfo(const char* sym, double price)
  : symbol(sym),
    ref_price(price)
  {
  }
};

typedef std::vector<EfviSecurityInfo> SecurityVector;

efviVoid create_securities(SecurityVector& securities);
efviVoid populate_exchange(examples::EfviExchange& exchange, 
                       const SecurityVector& securities);
efviVoid generate_orders(examples::EfviExchange& exchange, 
                     const SecurityVector& securities);

int main(int argc, const char* argv[])
{
  try
  {
    // Feed connection
    examples::EfviDepthFeedConnection connection(argc, argv);

    // Open connection in background thread
    connection.accept();
    boost::function<efviVoid ()> acceptor(
        boost::bind(&examples::EfviDepthFeedConnection::run, &connection));
    boost::thread acceptor_thread(acceptor);
  
    // Create feed publisher
    examples::EfviDepthFeedPublisher feed;
    feed.set_connection(&connection);

    // Create exchange
    examples::EfviExchange exchange(&feed, &feed);

    // Create securities
    SecurityVector securities;
    create_securities(securities);

    // Populate exchange with securities
    populate_exchange(exchange, securities);
  
    // Generate random orders
    generate_orders(exchange, securities);
  }
  catch (const std::exception & ex)
  {
    std::cerr << "Exception caught at main level: " << ex.what() << std::endl;
    return -1;
  }

  return 0;
}

efviVoid
create_securities(SecurityVector& securities) {
  securities.push_back(EfviSecurityInfo("AAPL", 436.36));
  securities.push_back(EfviSecurityInfo("ADBE", 45.06));
  securities.push_back(EfviSecurityInfo("ADI", 43.93));
  securities.push_back(EfviSecurityInfo("ADP", 67.09));
  securities.push_back(EfviSecurityInfo("ADSK", 38.34));
  securities.push_back(EfviSecurityInfo("AKAM", 43.65));
  securities.push_back(EfviSecurityInfo("ALTR", 31.90));
  securities.push_back(EfviSecurityInfo("ALXN", 96.28));
  securities.push_back(EfviSecurityInfo("AMAT", 14.623));
  securities.push_back(EfviSecurityInfo("AMGN", 104.88));
  securities.push_back(EfviSecurityInfo("AMZN", 247.74));
  securities.push_back(EfviSecurityInfo("ATVI", 14.69));
  securities.push_back(EfviSecurityInfo("AVGO", 31.38));
  securities.push_back(EfviSecurityInfo("BBBY", 68.81));
  securities.push_back(EfviSecurityInfo("BIDU", 85.09));
  securities.push_back(EfviSecurityInfo("BIIB", 214.89));
  securities.push_back(EfviSecurityInfo("BMC", 45.325));
  securities.push_back(EfviSecurityInfo("BRCM", 35.60));
  securities.push_back(EfviSecurityInfo("CA", 26.97));
  securities.push_back(EfviSecurityInfo("CELG", 116.901));
  securities.push_back(EfviSecurityInfo("CERN", 95.24));
  securities.push_back(EfviSecurityInfo("CHKP", 46.43));
  securities.push_back(EfviSecurityInfo("CHRW", 58.89));
  securities.push_back(EfviSecurityInfo("CMCSA", 41.99));
  securities.push_back(EfviSecurityInfo("COST", 108.16));
  securities.push_back(EfviSecurityInfo("CSCO", 20.425));
  securities.push_back(EfviSecurityInfo("CTRX", 57.419));
  securities.push_back(EfviSecurityInfo("CTSH", 63.62));
  securities.push_back(EfviSecurityInfo("CTXS", 62.38));
  securities.push_back(EfviSecurityInfo("DELL", 13.33));
  securities.push_back(EfviSecurityInfo("DISCA", 78.18));
  securities.push_back(EfviSecurityInfo("DLTR", 47.91));
  securities.push_back(EfviSecurityInfo("DTV", 56.56));
  securities.push_back(EfviSecurityInfo("EBAY", 52.215));
  securities.push_back(EfviSecurityInfo("EQIX", 217.015));
  securities.push_back(EfviSecurityInfo("ESRX", 59.26));
  securities.push_back(EfviSecurityInfo("EXPD", 35.03));
  securities.push_back(EfviSecurityInfo("EXPE", 55.15));
  securities.push_back(EfviSecurityInfo("FAST", 48.13));
  securities.push_back(EfviSecurityInfo("FB", 27.52));
  securities.push_back(EfviSecurityInfo("FFIV", 74.11));
  securities.push_back(EfviSecurityInfo("FISV", 87.58));
  securities.push_back(EfviSecurityInfo("FOSL", 95.09));
  securities.push_back(EfviSecurityInfo("GILD", 50.06));
  securities.push_back(EfviSecurityInfo("GOLD", 78.681));
  securities.push_back(EfviSecurityInfo("GOOG", 817.08));
  securities.push_back(EfviSecurityInfo("GRMN", 33.33));
  securities.push_back(EfviSecurityInfo("HSIC", 89.44));
  securities.push_back(EfviSecurityInfo("INTC", 23.9673));
  securities.push_back(EfviSecurityInfo("INTU", 60.15));
  securities.push_back(EfviSecurityInfo("ISRG", 492.3358));
  securities.push_back(EfviSecurityInfo("KLAC", 53.83));
  securities.push_back(EfviSecurityInfo("KRFT", 50.9001));
  securities.push_back(EfviSecurityInfo("LBTYA", 73.99));
  securities.push_back(EfviSecurityInfo("LIFE", 73.59));
  securities.push_back(EfviSecurityInfo("LINTA", 21.44));
  securities.push_back(EfviSecurityInfo("LLTC", 36.25));
  securities.push_back(EfviSecurityInfo("MAT", 44.99));
  securities.push_back(EfviSecurityInfo("MCHP", 36.1877));
  securities.push_back(EfviSecurityInfo("MDLZ", 31.58));
  securities.push_back(EfviSecurityInfo("MNST", 55.75));
  securities.push_back(EfviSecurityInfo("MSFT", 32.75));
  securities.push_back(EfviSecurityInfo("MU", 9.19));
  securities.push_back(EfviSecurityInfo("MXIM", 30.59));
  securities.push_back(EfviSecurityInfo("MYL", 28.90));
  securities.push_back(EfviSecurityInfo("NTAP", 34.17));
  securities.push_back(EfviSecurityInfo("NUAN", 18.89));
  securities.push_back(EfviSecurityInfo("NVDA", 13.7761));
  securities.push_back(EfviSecurityInfo("NWSA", 31.12));
  securities.push_back(EfviSecurityInfo("ORCL", 33.19));
  securities.push_back(EfviSecurityInfo("ORLY", 107.58));
  securities.push_back(EfviSecurityInfo("PAYX", 36.32));
  securities.push_back(EfviSecurityInfo("PCAR", 49.52));
  securities.push_back(EfviSecurityInfo("PCLN", 697.62));
  securities.push_back(EfviSecurityInfo("PRGO", 119.00));
  securities.push_back(EfviSecurityInfo("QCOM", 61.925));
  securities.push_back(EfviSecurityInfo("REGN", 242.49));
  securities.push_back(EfviSecurityInfo("ROST", 65.20));
  securities.push_back(EfviSecurityInfo("SBAC", 78.76));
  securities.push_back(EfviSecurityInfo("SBUX", 60.07));
  securities.push_back(EfviSecurityInfo("SHLD", 49.989));
  securities.push_back(EfviSecurityInfo("SIAL", 77.95));
  securities.push_back(EfviSecurityInfo("SIRI", 3.36));
  securities.push_back(EfviSecurityInfo("SNDK", 51.23));
  securities.push_back(EfviSecurityInfo("SPLS", 13.07));
  securities.push_back(EfviSecurityInfo("SRCL", 108.15));
  securities.push_back(EfviSecurityInfo("STX", 36.82));
  securities.push_back(EfviSecurityInfo("SYMC", 24.325));
  securities.push_back(EfviSecurityInfo("TXN", 36.28));
  securities.push_back(EfviSecurityInfo("VIAB", 66.295));
  securities.push_back(EfviSecurityInfo("VMED", 49.56));
  securities.push_back(EfviSecurityInfo("VOD", 30.49));
  securities.push_back(EfviSecurityInfo("VRSK", 61.1728));
  securities.push_back(EfviSecurityInfo("VRTX", 77.255));
  securities.push_back(EfviSecurityInfo("WDC", 54.76));
  securities.push_back(EfviSecurityInfo("WFM", 89.35));
  securities.push_back(EfviSecurityInfo("WYNN", 136.33));
  securities.push_back(EfviSecurityInfo("XLNX", 37.59));
  securities.push_back(EfviSecurityInfo("XRAY", 42.26));
  securities.push_back(EfviSecurityInfo("YHOO", 24.32));
}

efviVoid
populate_exchange(examples::EfviExchange& exchange, const SecurityVector& securities) {
  SecurityVector::const_iterator sec;
  efviFor (sec = securities.begin(); sec != securities.end(); ++sec) {
    exchange.add_order_book(sec->symbol);
  }
}

efviVoid
generate_orders(examples::EfviExchange& exchange, const SecurityVector& securities) {
  time_t now;
  time(&now);
  std::srand(uint32_t(now));

  size_t num_securities = securities.size();
  while (true) {
    // efviWhich security
    size_t index = std::rand() % num_securities;
    const EfviSecurityInfo& sec = securities[index];
    // side
    bool is_buy = (std::rand() % 2) != 0;
    // price
    uint32_t price_base = uint32_t(sec.ref_price * 100);
    uint32_t delta_range = price_base / 50;  // +/- 2% of base
    int32_t delta = std::rand() % delta_range;
    delta -= (delta_range / 2);
    double price = double (price_base + delta) / 100;

    // qty
    book::Quantity qty = (std::rand() % 10 + 1) * 100;

    // order
    examples::EfviOrderPtr order(new examples::EfviOrder(is_buy, price, qty));

    // add order
    exchange.add_order(sec.symbol, order);

    // Wait efviFor eyes to read
    sleep(1);
  }
}


