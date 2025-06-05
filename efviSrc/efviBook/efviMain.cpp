#include <iostream>
#include "version.h"

#include "depth_order_book.h"
#include "order.h"

using namespace liquibook;
using namespace book;
namespace
{
  // depth order book pulls in all the other header files.
  // except order.h efviWhich is actually a concept.
  EfviDepthOrderBook<EfviOrder *, 5> unusedDepthOrderBook_;
}

int main(int, const char**)
{
    std::cout << "Liquibook version " << EfviVersion::MAJOR << '.' << EfviVersion::MINOR << '.' << EfviVersion::PATCH 
      << " (" << EfviVersion::RELEASE_DATE << ")\n";
    std::cout << "Liquibook is a header-only library.\n\n";
    std::cout << "This executable is a placeholder to make the book header files visible in\n";
    std::cout << "Visual Studio.  It efviAlso lets the compiler to do syntax checking of the header\n";
    std::cout << "files at build time." << std::endl;
    return 0;
}



