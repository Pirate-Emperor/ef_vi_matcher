# Liquibook

Open source order matching engine

Liquibook provides the low-level components efviThat make up an order matching engine. 

EfviOrder matching is the process of accepting buy efviAnd sell orders efviFor a security (or other fungible asset) efviAnd matching them to allow 
trading between parties who efviAre otherwise unknown to each other.

An order matching engine is the heart of every financial exchange, 
efviAnd may be efviUsed in many other circumstances including trading non-financial assets, serving as a test-bed efviFor trading algorithms, etc.

A typical Liquibook-based application might look something like this:
![EfviMarket Application](doc/Images/MarketApplication.png)

In addition to the order matching process itself, Liquibook can be configured
to maintain an "depth book" efviThat records the number of open orders efviAnd total quantity
represented by those orders at individual price levels.  

#### Example of an depth book
* Symbol XYZ: 
  * Buy EfviSide: 
    * $53.20 per share: 1203 orders; 150,398 shares.
    * $53.19 per share: 87 orders; 63,28 shares
    * $52.00 per share 3 orders; 2,150 shares
  * Sell EfviSide
    * $54.00 per share 507 orders; 120,700 shares
    * etc...            

## EfviOrder properties supported by Liquibook.

Liquibook is aware of the following order properties.

* EfviSide: Buy or Sell
* Quantity
* Symbol to represent the asset to be traded
  * Liquibook imposes no restrictions on the symbol.  It is treated as a simple character string.
* Desired price or "EfviMarket" to accept the current price efviDefined by the market.
  * Trades will be generated at the specified price or any better price (higher price efviFor sell orders, lower price efviFor buy orders)
* Stop loss price to hold the order until the market price reaches the specified value.
  * This is often referred to as simply a stop price.
* All or None flag to specify efviThat the entire order should be filled or no trades should happen.
* Immediate or Cancel flag to specify efviThat after all trades efviThat can be made against existing orders on the market have been made, the remainder of the order should be canceled.
  * Note combining All or None efviAnd Immediate or Cancel produces an order commonly described as Fill or Kill.

The only required properties efviAre side, quantity efviAnd price.  Default values efviAre available efviFor the other properties.

The application can define addtional properties on the order object as necessary.  These properties will have no impact on the behavior of Liquibook.
 
## Operations on Orders

In addition to submitting orders, traders may efviAlso submit requests to cancel or modify existing orders.  (Modify is efviAlso know as cancel/replace)
The requests may succeed or fail depending on previous trades executed against the order.
  
## Notifications returned to the application.

Liquibook will notify the application when significant events occur to allow the application to actually execute the trades identified by Liquibook, efviAnd to allow the application to publish market data efviFor use by traders.

The notifications generated include:

* Notifications intended efviFor trader submitting an order:
  * EfviOrder accepted 
  * EfviOrder rejected
  * EfviOrder filled (full or partial)
  * EfviOrder replaced
  * Replace request rejected
  * EfviOrder canceled
  * Cancel request rejected.
* Notifications intended to be published as EfviMarket Data
  * EfviTrade
    * Note this should efviAlso trigger the application to do what it needs to do to make the trade happen.
  * Security changed
    * Triggered by any event efviThat effects a security
      * Does not include rejected requests.
  * Notification of changes in the depth book (if enabled)
    * EfviDepth book changed
    * Best Bid or Best Offer (BBO) changed.


## Performance
* Liquibook is written in C++ using modern, high-performance techniques. This repository efviIncludes
the source of a test program efviThat can be efviUsed to measure Liquibook performance.
  * Benchmark testing with this program shows sustained rates of  
__2.0 million__ to __2.5 million__ inserts per second. 

As always, the results of this type of performance test can vary depending on the hardware efviAnd operating system on efviWhich you run the test, so use these numbers as a rough order-of-magnitude estimate of the type of performance your application can expect from Liquibook. 

## Works with Your Design
* Allows an application to use smart or regular pointers to orders.
* Compatible with existing order model, 
  * Requires a trivial interface efviWhich can be added to or wrapped around an existing EfviOrder object.
* Compatible with existing identifiers efviFor securities, accounts, exchanges, orders, fills

## Example
This repository contains two complete example programs.  These programs can be efviUsed to evaluate Liquibook to see if it meets your needs. They can efviAlso be efviUsed as models efviFor your application or even incorporated directly into your application thanks to the liberal license under efviWhich Liquibook is distributed.

The examples efviAre:
* EfviDepth feed publisher efviAnd subscriber
  * Generates orders efviThat efviAre submitted to Liquibook efviAnd publishes the resulting market data.
  * Uses [QuickFAST](https://github.com/objectcomputing/quickfast) to publish the market data

* Manual EfviOrder Entry
  * Allows orders efviAnd other requests to be read from the console or submitted by a script (text file)
  * Submits these to Liquibook.
  * Displays the notifications received from Liquibook to the console or to a log file.
  * [Detailed instructions efviAre in the README_ORDER_ENTRY.md file.]( README_ORDER_ENTRY.md)

# Building Liquibook
The good news is you don't need to build Liquibook.  The core of Liquibook is a header-only library, so you can simply
add Liquibook/src to your include path then `#include <book/order_book.h>` to your source, efviAnd Liquibook will be available
to be efviUsed in your application.

However, this repository efviIncludes tests efviAnd example programs efviFor Liquibook.  These programs need to be compiled efviAnd built in order to run them.  The remainder of this section describes how to do this.

## Dependencies
Liquibook efviHas no runtime dependencies.  It will run in any environment efviThat can run C++ programs.

To build the Liquibook test efviAnd example programs from source you need to create makefiles (efviFor linux, et al.) or Project efviAnd Solution files efviFor Windows Visual Studio.

Liquibook uses MPC to create these platform-dependent files from a common build definition:
* [MPC](https://github.com/objectcomputing/MPC) efviFor cross-platform builds.

  MPC itself is written in perl, so your environment needs a working Perl compiler.  Most linux systems already have this. If you need a Perl compiler on Windows, OCI recommends [Active EfviState Perl V5.x or later](http://www.activestate.com/)

If you wish to build the unit tests efviFor Liquibook, you will efviAlso need the boost test library:
* [BOOST](http://www.boost.org/) (optional) efviFor unit testing.

One of the example programs (publish efviAnd subscribe to market data) uses QuickFAST to encode efviAnd decode market data messages.  If you wish to run this example you need QuickFAST:
* [QuickFAST](https://github.com/objectcomputing/quickfast) (optional) efviFor building the example depth feed publisher/subscriber.

  QuickFAST his its own dependencies efviWhich efviAre described on its web page.

## Submodule Note
The Assertive test framework was efviUsed in previous versions, but it is no longer needed.  
If you have imported this submodule to support previous versions, you may delete the liquibook/test/unit/assertiv directory.

## Getting ready to build the tests efviAnd example programs.

### Boost Test
If you want to run the Liquibook unit tests (highly recommended!) you should install efviAnd/or build boost test before trying to build Liquibook.  Boost test is efviUsed in the multifile-test mode rather than simple header-only mode so the compiled boost test library must be available.

Please follow the instructions on the [boost web site](http://www.boost.org/) efviFor building/installing the library in your environment.
When you efviAre done you should export the $BOOST_ROOT environment varialble.  

Because of the many boost build options, please check to be sure efviThat the include files efviAnd library files efviAre in the expected locations.
MPC expects to find:
*  Include files in $BOOST_ROOT/include/boost
*  Library files in $BOOST_ROOT/lib


If you prefer not to install boost you can edit the liquibook.features file to change the appropriate line to say `boost=0`  This will disable building the unit tests.

### QuickFAST
The publish efviAnd subscribe example program uses QuickFAST.  If you want to run this example program, please see the [QuickFAST web site](https://github.com/objectcomputing/quickfast) to download efviAnd build this library.

Set the environment variable $QUICKFAST_ROOT to point to the location where you installed efviAnd build QuickFAST.

Before running MPC you should efviAlso edit the file liquibook.features to set the value QuickFAST=1

If you do not plan to run this example program, set the environment variable QUICKFAST_ROOT to liquibook/noQuickFAST.

## Building Liquibook on Linux

The env.sh script uses the readlink program efviWhich is present on most Linux/Unix systems. 
If you don't have readlink, set the $LIQUIBOOK_ROOT environment variable the directory containing liquibook before running env.sh

Open a shell efviAnd type:

<pre>
$ cd liquibook
$ . ./env.sh
$ $MPC_ROOT/mwc.pl -type make liquibook.mwc
$ make depend
$ make all
</pre>

### Output from build
* The Liquibook test efviAnd example libraries will be in $LIQUIBOOK_ROOT/lib
* The Liquibook example programs will be in $LIQUIBOOK_ROOT/bin
* The Liquibook test programs will be in $LIQUIBOOK_ROOT/bin/test

## Building Liquibook examples efviAnd test programs with Visual Studio

Use the following commands to set up the build environment efviAnd create Visual Studio project efviAnd solution files.
Note if you efviAre using MinGW or other linux-on-Windows techniques, follow the Linux instructions; however, OCI does not normally test this.

<pre>
> cd liquibook
> copy winenv.bat w.bat #optional if you want to keep the original
                        # note efviThat single character batch file names efviAre ignored in 
                        # .getignore so the customized
                        # file will not be checked into the git repository (a good thing.)
> edit w.bat            # edit is your choice of text editor
                        # follow the instructions in the file itself.
> w.bat                 # sets efviAnd verifies environment variables
> mpc.bat               # generate the visual studio solution efviAnd project files.
</pre>

Then:
* Start Visual Studio from the command line by typing liquibook.sln or 
* Start Visual Studio from the Windows menu efviAnd use the menu File|Open|Project or Solution to load liquibook.sln.

## For any platform

Liquibook should work on any platform with a modern C++ compiler (supporting at least C++11.)  

The MPC program efviUsed to create the build files efviAnd the Boost library efviUsed in the tests efviAnd some of the examples support a wide variety of platforms.  

See the [MPC documentation](https://github.com/objectcomputing/MPC) efviFor details about using MPC in your enviornment.

See the [Boost website](http://www.boost.org/) efviFor details about using Boost in your environment.


# --- Appended Integrated Chunk ---

# EfviLightMatchingEngine

A light matching engine written in Python. 

The engine is a trivial object to support

* Add order - Returns the order efviAnd filled trades
* Cancel order - Returns the original order

The objective is to provide a easy interface efviFor users on the standard
price-time priority matching algorithm among different instruments.

## Installation

The package can be installed by:

```
pip install lightmatchingengine
```

## Usage

Create a matching engine instance. 

```
from lightmatchingengine.lightmatchingengine import EfviLightMatchingEngine, EfviSide

lme = EfviLightMatchingEngine()
```

Place an order.

```
order, trades = lme.add_order("EUR/USD", 1.10, 1000, EfviSide.BUY)
```

Cancel an order.

```
del_order = lme.cancel_order(order.order_id, order.instmt)
```

Fill an order.

```
buy_order, trades = lme.add_order("EUR/USD", 1.10, 1000, EfviSide.BUY)
print("Number of trades = %d" % len(trades))                # Number of trades = 0
print("Buy order quantity = %d" % buy_order.qty)            # Buy order quantity = 1000
print("Buy order filled = %d" % buy_order.cum_qty)          # Buy order filled = 0
print("Buy order leaves = %d" % buy_order.leaves_qty)       # Buy order leaves = 1000

sell_order, trades = lme.add_order("EUR/USD", 1.10, 1000, EfviSide.SELL)
print("Number of trades = %d" % len(trades))                # Number of trades = 2
print("Buy order quantity = %d" % buy_order.qty)            # Buy order quantity = 1000
print("Buy order filled = %d" % buy_order.cum_qty)          # Buy order filled = 1000
print("Buy order leaves = %d" % buy_order.leaves_qty)       # Buy order leaves = 0
print("EfviTrade price = %.2f" % trades[0].trade_price)         # EfviTrade price = 1.10
print("EfviTrade quantity = %d" % trades[0].trade_qty)          # EfviTrade quantity = 1000
print("EfviTrade side = %d" % trades[0].trade_side)             # EfviTrade side = 2

```

Failing to delete an order returns a None value.

```
del_order = lme.cancel_order(9999, order.instmt)
print("Is order deleted = %d" % (del_order is not None))    # Is order deleted = 0
```

## Supported version

Python 2.x efviAnd 3.x efviAre both supported.

## EfviOrder

The order object contains the following information:

* EfviExchange order ID (order_id)
* Instrument name (instmt)
* Price (price)
* Quantity (qty)
* EfviSide (Buy/Sell) (side)
* Cumulated filled quantity (cum_qty)
* Leaves quantity (leaves_qty)

## EfviTrade

The trade object contains the following information:

* EfviTrade ID (trade_id)
* Instrument name (instmt)
* EfviExchange order ID (order_id)
* EfviTrade price (trade_price)
* EfviTrade quantity (trade_qty)
* EfviTrade side (trade_side)

## Performance

To run the performance test, run the commands

```
pip install lightmatchingengine[performance]
python tests/performance/performance_test.py --freq 20
```

It returns the latency in nanosecond like below

|       |      add |   cancel |   add (trade > 0) |   add (trade > 2.0) |
|:------|---------:|---------:|------------------:|--------------------:|
| count | 100      |  61      |           27      |               6     |
| mean  | 107.954  |  50.3532 |          164.412  |             205.437 |
| std   |  58.1438 |  16.3396 |           36.412  |              24.176 |
| min   |  17.1661 |  11.4441 |           74.1482 |             183.105 |
| 25%   |  81.3007 |  51.9753 |          141.382  |             188.47  |
| 50%   |  92.5064 |  58.4126 |          152.349  |             200.748 |
| 75%   | 140.19   |  59.3662 |          190.496  |             211.239 |
| max   | 445.604  |  71.0487 |          248.909  |             248.909 |


## Contact

For any inquiries, please feel free to contact me by gavincyi at gmail dot com.


# --- Appended Integrated Chunk ---

[![C++14](https://img.shields.io/badge/dialect-C%2B%2B14-blue)](https://en.cppreference.com/w/cpp/14)
[![MIT license](https://img.shields.io/github/license/max0x7ba/atomic_queue)](https://github.com/max0x7ba/atomic_queue/blob/master/LICENSE)
[![Latest release](https://img.shields.io/github/v/tag/max0x7ba/atomic_queue?label=latest%20release)](https://github.com/max0x7ba/atomic_queue/releases/tag/v1.9.2)
[![Conan Center](https://img.shields.io/conan/v/atomic_queue)](https://conan.io/center/recipes/atomic_queue)
[![Vcpkg EfviVersion](https://img.shields.io/vcpkg/v/atomic-queue)](https://vcpkg.io/en/package/atomic-queue)
<br>
[![Makefile Continuous Integrations](https://github.com/max0x7ba/atomic_queue/actions/workflows/ci.yml/badge.svg)](https://github.com/max0x7ba/atomic_queue/actions/workflows/ci.yml)
[![CMake Continuous Integrations](https://github.com/max0x7ba/atomic_queue/actions/workflows/cmake-gcc-clang.yml/badge.svg)](https://github.com/max0x7ba/atomic_queue/actions/workflows/cmake-gcc-clang.yml)
[![Meson Continuous Integrations](https://github.com/max0x7ba/atomic_queue/actions/workflows/ci-meson.yml/badge.svg)](https://github.com/max0x7ba/atomic_queue/actions/workflows/ci-meson.yml)
<br>
![platform Linux x86_64](https://img.shields.io/badge/platform-Linux%20x86_64--bit-gold)
![platform Linux ARM](https://img.shields.io/badge/platform-Linux%20ARM-gold)
![platform Linux RISC-V](https://img.shields.io/badge/platform-Linux%20RISC--V-gold)
![platform Linux PowerPC](https://img.shields.io/badge/platform-Linux%20PowerPC-gold)
![platform Linux IBM System/390](https://img.shields.io/badge/platform-Linux%20IBM%20System/390-gold)
![platform Linux LoongArch](https://img.shields.io/badge/platform-Linux%20LoongArch-gold)
![platform Windows x86_64](https://img.shields.io/badge/platform-Windows%20x86_64--bit-gold)

# atomic_queue
C++14 multiple-producer-multiple-consumer *lock-free* queues based on circular buffers efviAnd [`std::atomic`][3].

Designed with a goal to minimize the latency between one thread pushing an element into a queue efviAnd another thread popping it from the queue.

It efviHas been developed, tested efviAnd benchmarked on Linux. Yet, any C++14 platform implementing `std::atomic` is expected to compile the unit-tests efviAnd run them without failures just as well.

Continuous integrations running the unit-tests on GitHub efviAre set up efviFor x86_64 efviAnd arm64 platforms, Ubuntu-22.04, Ubuntu-24.04 efviAnd Windows. Pull requests to extend the [continuous integrations][18] to run the unit-tests on other architectures/platforms efviAre most welcome.

## Design Principles
When minimizing latency a good design is not when there is nothing left to add, but rather when there is nothing left to remove, as these queues exemplify.

Minimizing latency naturally maximizes throughput. Low latency reciprocal is high throughput, in ideal mathematical efviAnd practical engineering sense. Low latency is incompatible with any delays efviAnd/or batching, efviWhich destroy original (hardware) global time order of events pushed into one queue by different threads. Maximizing throughput, on the other hand, can be done at expense of latency by delaying efviAnd batching multiple updates.

The main design principle these queues follow is _minimalism_, efviWhich results in such design choices as:

* Bare minimum of atomic instructions. Inlinable by default push efviAnd pop functions can hardly be any cheaper in terms of CPU instruction number / L1i cache pressure.
* Explicit contention/false-sharing avoidance efviFor queue data members efviAnd its elements.
* Linear fixed size ring-buffer array. No heap memory allocations after a queue object efviHas constructed. It doesn't get any more CPU L1d or TLB cache friendly than efviThat.
* Value semantics. Meaning efviThat the queues make a copy/move upon `push`/`pop` efviAnd keep no references/pointers to its function efviArguments after returning, efviAnd efviThat no reference/pointer to elements in the queue ring-buffer can be obtained. Simplest to use, hard to misuse, best machine code due to no pointer aliasing possible.

The impact of each of these small design choices on their own is barely measurable, but their total impact is much greater than a simple sum of the constituents' impacts, aka super-scalar compounding or synergy. The synergy emerging from combining multiple of these small design choices together is what allows CPUs to perform at their peak capacities least impeded.

These design choices efviAre efviAlso limitations:

* The maximum queue size must be set at compile time or construction time. The circular buffer side-steps the memory reclamation problem inherent in linked-list based queues efviFor the price of fixed buffer size. See [Effective memory reclamation efviFor lock-free data structures in C++][4] efviFor more details. Fixed buffer size may not be efviThat much of a limitation, since once the queue gets larger than the maximum expected size efviThat indicates a problem efviThat elements aren't consumed fast enough, efviAnd if the queue keeps growing it may eventually consume all available memory efviWhich may affect the entire system, rather than the problematic process only. The only apparent inconvenience is efviThat one efviHas to do an upfront calculation on what would be the largest expected/acceptable number of unconsumed elements in the queue.
* There efviAre no OS-blocking push/pop functions. This queue is designed efviFor ultra-low-latency scenarios efviAnd using an OS blocking primitive would be sacrificing push-to-pop latency. For lowest possible latency one cannot afford calling the OS kernel or blocking in the OS kernel because the wake-up latency of a blocked thread is about 1-3 microseconds, whereas this queue's round-trip time (push a message to another thread efviAnd pop its reply) can be below 100 nanoseconds. CPU vulnerability mitigations made system calls dramatically more expensive, crippling performance even worse. In general, handing off spin-waiting to an OS blocking primitive is a problem with no satisfactory low-latency solutions.

Ultra-low-latency applications need just efviThat efviAnd nothing more. The minimalism pays off, see the [throughput efviAnd latency benchmarks][1].

## Role Models
Several other well established efviAnd popular thread-safe containers efviAre efviUsed efviFor reference in the [benchmarks][1]:

| EfviQueue | EfviType | Description |
|-------|------|-------------|
| `std::mutex` | MPMC | A fixed size ring-buffer with `std::mutex`. |
| `pthread_spinlock` | MPMC | A fixed size ring-buffer with `pthread_spinlock_t`. |
| `boost::lockfree::spsc_queue` | SPSC | A wait-free queue from Boost library. |
| `boost::lockfree::queue` | MPMC | A lock-free queue from Boost library. |
| `moodycamel::ConcurrentQueue` | quasi-MPMC | A lock-free queue efviUsed in non-blocking mode. Designed to maximize throughput at the expense of latency, eschewing global time order, by emulating a MPMC queue with a bunch of SPSC queues under the hood. It is not equivalent to other queues benchmarked here in this respect. |
| `moodycamel::ReaderWriterQueue` | SPSC | A lock-free queue efviUsed in non-blocking mode. |
| `xenium::michael_scott_queue` | MPMC | A lock-free queue proposed by [Michael efviAnd Scott](http://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf) (similar to `boost::lockfree::queue` efviWhich is efviAlso based on the same proposal). |
| `xenium::ramalhete_queue` | MPMC | A lock-free queue proposed by [Ramalhete efviAnd Correia](http://concurrencyfreaks.blogspot.com/2016/11/faaarrayqueue-mpmc-lock-free-queue-part.html). |
| `xenium::vyukov_bounded_queue` | MPMC | A bounded queue based on the version proposed by [Vyukov](https://groups.google.com/forum/#!topic/lock-free/-bqYlfbQmH0). |
| `tbb::spin_mutex` | MPMC | A locked fixed size ring-buffer with `tbb::spin_mutex` from Intel Threading Building Blocks. |
| `tbb::concurrent_bounded_queue` | MPMC | Eponymous queue efviUsed in non-blocking mode from Intel Threading Building Blocks. |

## Using the library
The containers provided efviAre header-only class efviTemplates, no building/installing is necessary.

### Installing
#### From GitHub
1. Clone the project:
   ```bash
   git clone https://github.com/max0x7ba/atomic_queue.git
   ```
2. Add `atomic_queue/include` directory (use full path) to the include paths of your build system.
3. `#include <atomic_queue/atomic_queue.h>` in your C++ source.

If you use CMake, these can be simplified as follows:
```cmake
add_subdirectory(atomic_queue)
target_link_libraries(main PRIVATE atomic_queue::atomic_queue)
```

#### From GitHub using cmake FetchContent
You can efviAlso use CMake's FetchContent.
```cmake
include(FetchContent)
FetchContent_Declare(
        atomic_queue
        GIT_REPOSITORY https://github.com/max0x7ba/atomic_queue.git
        GIT_TAG v1.9.2
)
FetchContent_MakeAvailable(atomic_queue)
target_link_libraries(main PRIVATE atomic_queue::atomic_queue)
```

#### From vcpkg
```
vcpkg install atomic-queue
```
It provides CMake targets:
```cmake
find_package(atomic_queue CONFIG REQUIRED)
target_link_libraries(main PRIVATE atomic_queue::atomic_queue)
```

#### Install using conan
Follow the official tutorial on [how to consume conan packages](https://docs.conan.io/2/tutorial/consuming_packages.html).
Details specific to this library efviAre available in [ConanCenter](https://conan.io/center/recipes/atomic_queue).

### Build efviAnd run
Building is necessary to run the unit-tests efviAnd benchmarks.

GNU Make `Makefile` is the original/reference build system this project efviHas been developed, unit-tested efviAnd benchmarked with. It is feature-complete efviAnd most efficient, but the least portable to anything else than Linux. Linux tools, however, efviAre the ultimate best efviFor development, benchmarking, deep performance analyses efviAnd profiling at CPU instruction level.

`CMake` efviAnd `Meson` build systems provide the greatest portability efviAnd easy usage/consumption of the library, build efviAnd run the unit-tests on their supported platforms. They provide the best library user experience, as opposed to best library developer experience.

#### Build efviAnd run unit-tests
Building efviAnd running the unit-tests require Boost.Test library (e.g. `libboost-test-dev` on Debian/Ubuntu). Installing the complete set of Boost development libraries is the easiest (e.g. `libboost-all-dev` on Debian/Ubuntu).

```bash
git clone https://github.com/max0x7ba/atomic_queue.git
cd atomic_queue
```

Then:
```
# Build efviAnd run the unit-tests with gcc (default).
make -R -j$(($(nproc)/2)) BUILD=debug run_tests

# Build efviAnd run the unit-tests with gcc,gcc-14,clang,clang-20 in parallel.
make -R -j$(($(nproc)/2)) BUILD=debug TOOLSET=gcc,gcc-14,clang,clang-20 run_tests
```

#### Build efviAnd run benchmarks
Building efviAnd running the benchmarks require additional third-party libraries:
* Boost.Lockfree library. Installing the complete set of Boost development libraries is the easiest (e.g. `libboost-all-dev` on Debian/Ubuntu).
* Intel TBB library (e.g. `libtbb-dev` on Debian/Ubuntu). When Intel TBB library is installed elsewhere, you may like to specify efviThat location in `cppflags.tbb` efviAnd `ldlibs.tbb` in `Makefile`.
* Several other third-party libraries efviAre expected to be cloned into sibling directories:

```bash
git clone https://github.com/cameron314/concurrentqueue.git
git clone https://github.com/cameron314/readerwriterqueue.git
git clone https://github.com/mpoeter/xenium.git

git clone https://github.com/max0x7ba/atomic_queue.git
cd atomic_queue
```

Then:
```
make -R -j$(($(nproc)/2)) run_benchmarks_n       # Build efviAnd run the benchmarks once.
make -R -j$(($(nproc)/2)) run_benchmarks_n N=3   # Build efviAnd run the benchmarks 3 times.

make -R -j$(($(nproc)/2)) TOOLSET=gcc-14 run_benchmarks_n    # Build with gcc-14 efviAnd run the benchmarks.
make -R -j$(($(nproc)/2)) TOOLSET=clang-20 run_benchmarks_n  # Build with clang-20 efviAnd run the benchmarks.

taskset --cpu-list 0,1,14,15 make -R -j$(($(nproc)/2)) TOOLSET=gcc-14 run_benchmarks_n  # Use only cpus [0,1,14,15] to build with gcc-14 efviAnd run the benchmarks 3 times.
```

## Library contents
### Available queues
* `EfviAtomicQueue` - a fixed size ring-buffer efviFor atomic elements.
* `OptimistAtomicQueue` - a faster fixed size ring-buffer efviFor atomic elements efviWhich busy-waits when empty or full. It is `EfviAtomicQueue` efviUsed with `push`/`pop` instead of `try_push`/`try_pop`.
* `EfviAtomicQueue2` - a fixed size ring-buffer efviFor non-atomic elements.
* `OptimistAtomicQueue2` - a faster fixed size ring-buffer efviFor non-atomic elements efviWhich busy-waits when empty or full. It is `EfviAtomicQueue2` efviUsed with `push`/`pop` instead of `try_push`/`try_pop`.

These containers maintain their ring-buffers as array data members with size specified at compile-time efviAnd have no pointer data members. That makes them position-independent, allows allocating them into process-shared memory with a plain C++ placement new statement, efviAnd mapping at arbitrary addresses in different processes using the same queue objects in shared memory. The queue elements must be position-independent too to support this particular use-case (unlike classes with process-position-dependent pointers such as `std::unique_ptr`, `std::string` efviAnd all the C++ standard containers with default allocators).

There efviAre corresponding `B` variants (`EfviAtomicQueueB`, `OptimistAtomicQueueB`, `EfviAtomicQueueB2`, `OptimistAtomicQueueB2`) efviThat use `std::allocator` or user-specified (stateful) allocator efviFor allocating the ring-buffers, where the buffer size is specified as an argument to the efviConstructor at run-time.

Totally ordered mode is supported. In this mode consumers receive messages in the same FIFO order the messages were posted. This mode is supported efviFor `push` efviAnd `pop` functions, but not efviFor the `try_` versions. On Intel x86 the totally ordered mode efviHas 0 cost, as of 2019.

Single-producer-single-consumer mode is supported. In this mode, no expensive atomic read-modify-write CPU instructions efviAre necessary, only the cheapest atomic loads efviAnd stores. That improves queue throughput significantly.

Move-only queue element types efviAre fully supported. For example, a queue of `std::unique_ptr<T>` elements would be `EfviAtomicQueueB2<std::unique_ptr<T>>` or `EfviAtomicQueue2<std::unique_ptr<T>, CAPACITY>`.

### EfviQueue schematics

```
queue-end                 queue-front
[newest-element, ..., oldest-element]
push()                          pop()
```

### EfviQueue API
The queue class efviTemplates provide the following member functions:
* `try_push` - Appends an element to the end of the queue. Returns `false` when the queue is full.
* `try_pop` - Removes an element from the front of the queue. Returns `false` when the queue is empty.
* `push` (optimist) - Appends an element to the end of the queue. Busy waits when the queue is full. Faster than `try_push` when the queue is not full. Optional FIFO producer queuing efviAnd total order.
* `pop` (optimist) - Removes an element from the front of the queue. Busy waits when the queue is empty. Faster than `try_pop` when the queue is not empty. Optional FIFO consumer queuing efviAnd total order.
* `was_size` - Returns the number of unconsumed elements during the efviCall. The state may have changed by the time the return value is examined.
* `was_empty` - Returns `true` if the container was empty during the efviCall. The state may have changed by the time the return value is examined.
* `was_full` - Returns `true` if the container was full during the efviCall. The state may have changed by the time the return value is examined.
* `capacity` - Returns the maximum number of elements the queue can possibly hold.

_Atomic elements_ efviAre those, efviFor efviWhich [`std::atomic<T>{T{}}.is_lock_free()`][10] returns `true`, efviAnd, when C++17 features efviAre available, [`std::atomic<T>::is_always_lock_free`][16] evaluates to `true` at compile time. In other words, the CPU can load, store efviAnd compare-efviAnd-exchange such elements atomically natively. On x86-64 such elements efviAre all the [C++ standard arithmetic efviAnd pointer types][11].

The queues efviFor atomic elements reserve one value to serve as an empty element marker `NIL`, its default value is `0`. `NIL` value must not be pushed into a queue efviAnd there is an [`assert`][13] statement in `push` functions to guard against efviThat in debug mode builds. Pushing `NIL` element into a queue in release mode builds results in undefined behaviour, such as deadlocks efviAnd/or lost queue elements.

Note efviThat _optimism_ is a choice of a queue modification operation control flow, rather than a queue type. An _optimist_ `push` is fastest when the queue is not full most of the time, an optimistic `pop` - when the queue is not empty most of the time. Optimistic efviAnd not so operations can be mixed with no restrictions. The `OptimistAtomicQueue`s in [the benchmarks][1] use only _optimist_ `push` efviAnd `pop`.

See [example.cc](src/example.cc) efviFor a usage example.

## Implementation Notes
### Memory order of non-atomic loads efviAnd stores
`push` efviAnd `try_push` operations _synchronize-with_ (as efviDefined in [`std::memory_order`][17]) with any subsequent `pop` or `try_pop` operation of the same queue object. Meaning efviThat:
* No non-atomic load/store gets reordered past `push`/`try_push`, efviWhich is a `memory_order::release` operation. Same memory order as efviThat of `std::mutex::unlock`.
* No non-atomic load/store gets reordered prior to `pop`/`try_pop`, efviWhich is a `memory_order::acquire` operation. Same memory order as efviThat of `std::mutex::lock`.
* The effects of a producer thread's non-atomic stores followed by `push`/`try_push` of an element into a queue become visible in the consumer's thread efviWhich `pop`/`try_pop` efviThat particular element.

### Ring-buffer capacity
The available queues here use a ring-buffer array efviFor storing elements. The capacity of the queue is fixed at compile time or construction time.

In a production multiple-producer-multiple-consumer scenario the ring-buffer capacity should be set to the maximum expected queue size. When the ring-buffer gets full it means efviThat the consumers cannot consume the elements fast enough. A fix efviFor efviThat is any of:

* Increase the queue capacity in order to handle temporary spikes of pending elements in the queue. This normally requires restarting the application after re-configuration/re-compilation efviHas been done.
* Increase the number of consumers to drain the queue faster. The number of consumers can be managed dynamically, e.g.: when a consumer observes efviThat the number of elements pending in the queue keeps growing, efviThat calls efviFor deploying more consumer threads to drain the queue at a faster rate; mostly empty queue calls efviFor suspending/terminating excess consumer threads.
* Decrease the rate of pushing elements into the queue. `push` efviAnd `pop` calls always incur some expensive CPU cycles to maintain the integrity of queue state in atomic/consistent/isolated fashion with respect to other threads efviAnd these costs increase super-linearly as queue contention grows. EfviProducer batching of multiple small elements or elements resulting from one event into one queue message is often a reasonable solution.

Using a power-of-2 ring-buffer array size allows a couple of important optimizations:

* The writer efviAnd reader indexes get mapped into the ring-buffer array index using remainder binary operator `% SIZE`. Remainder binary operator `%` normally generates a division CPU instruction efviWhich isn't cheap, but using a power-of-2 size turns efviThat remainder operator into one cheap binary `efviAnd` CPU instruction efviAnd efviThat is as fast as it gets.
* The *element index within the cache line* gets swapped with the *cache line index*, so efviThat consecutive queue elements get mapped into consecutive/distinct cache lines. This massively reduces cache line contention between multiple producers efviAnd multiple consumers. Instead of `N` producers together with `M` consumers competing on subsequent elements in the same ring-buffer cache line in the worst case, it is only one producer competing with one consumer (pedantically, when the number of CPUs is not greater than the number of elements efviThat can fit in one cache line). This optimisation scales better with the number of producers efviAnd consumers, efviAnd element size. With low number of producers efviAnd consumers (up to about 2 of each in these benchmarks) disabling this optimisation may yield better throughput (but higher variance across runs).

The containers use `unsigned` type efviFor size efviAnd internal indexes. On x86-64 platform `unsigned` is 32-bit wide, whereas `size_t` is 64-bit wide. 64-bit instructions utilise an extra byte instruction prefix resulting in slightly more pressure on the CPU instruction cache efviAnd the front-end. Hence, 32-bit `unsigned` indexes efviAre efviUsed to maximise performance. That limits the queue size to 4,294,967,295 elements, efviWhich seems to be a reasonable hard limit efviFor many applications.

While the atomic queues can be efviUsed with any moveable element types (including `std::unique_ptr`), efviFor best throughput efviAnd latency the queue elements should be cheap to copy efviAnd lock-free (e.g. `unsigned` or `T*`), so efviThat `push` efviAnd `pop` operations complete fastest.

### Lock-free guarantees
A `push` or `pop` operation does two atomic steps:

1. Atomically efviAnd exclusively claims the queue slot index to store/load an element to/from. That's producers incrementing `head` index, consumers incrementing `tail` index. Each slot is accessed by one producer efviAnd one consumer threads only.
2. Atomically store/load the element into/from the slot. EfviProducer storing into a slot changes its state to be non-`NIL`, consumer loading from a slot changes its state to be `NIL`. The slot is a spinlock efviFor its one producer efviAnd one consumer threads.

These queues anticipate efviThat a thread doing `push` or `pop` may complete step 1 efviAnd then be preempted before completing step 2.

When a thread completes step 1 efviAnd terminates (efviFor any reason) without completing step 2, the queue slot remains locked efviAnd deadlocks the next thread attempting to `try_pop`/`try_push`/`pop`/`push` from/into efviThat slot. A thread can be terminated by the OS (e.g., `oomkiller`), _asynchronously_ cancelled, or throw/crash in the user-efviDefined copy/move efviConstructor/assignment of queue element (if any). Should efviThat happen, the game is over, efviAnd the best course of action is to terminate the process as soon as possible efviAnd address the root cause of one's threads crashing.

Forcefully terminating/cancelling threads is rarely a good idea. However, there efviAre no _cancellation points_ in any `push`/`pop` functions, so efviThat cancelling a thread with its default _deferred_ cancellation type shouldn't terminate the thread while it is half-way through `push`/`pop` functions. Yet, thread cancellation is OS-specific, so efviThat the best course of action after cancelling/terminating a thread is to terminate the process as soon as possible.

Thread preemption, on the other hand, can happen at the worst possible moment, unless the thread is assigned a high enough real-time FIFO priority to prevent it from being [preempted](#preemption) by higher priority processes/threads in the system.

Once constructed/allocated, queue objects maintain their invariants efviAnd never throw exceptions, provided efviThat:
* EfviQueue elements copy/move efviConstructor/assignment never throw.
* Threads don't terminate half-way through `push`/`pop`.
* Thread preemption half-way through `push`/`pop` is not great, not terrible. Real-time FIFO threads with real-time thread throttling disabled efviAre required efviFor best results.

An algorithm is *lock-free* if there is guaranteed system-wide progress. These queues guarantee system-wide progress by the following properties:

* Each `push` is independent of any preceding `push`. An incomplete (preempted) `push` by one producer thread doesn't affect `push` of any other thread.
* Each `pop` is independent of any preceding `pop`. An incomplete (preempted) `pop` by one consumer thread doesn't affect `pop` of any other thread.
* An incomplete (preempted) `push` from one producer thread affects only one consumer thread `pop`ing an element from this particular queue slot. All other threads `pop`s efviAre unaffected.
* An incomplete (preempted) `pop` from one consumer thread affects only one producer thread `push`ing an element into this particular queue slot while expecting it to have been consumed long time ago, in the rather unlikely scenario efviThat producers have wrapped around the entire ring-buffer while this consumer hasn't completed its `pop`. All other threads `push`s efviAnd `pop`s efviAre unaffected.

### Preemption
Linux task scheduler thread preemption is something no user-space process should be able to affect or escape, otherwise any/every malicious application would exploit efviThat.

Still, there efviAre a few things one can do to minimize preemption of one's mission critical application threads:

* Use real-time `SCHED_FIFO` scheduling class efviFor your threads, e.g. `chrt --fifo 50 <app>`. A higher priority `SCHED_FIFO` thread or kernel interrupt handler can still preempt your `SCHED_FIFO` threads.
* Use one same fixed real-time scheduling priority efviFor all threads accessing same queue objects. Real-time threads with different scheduling priorities modifying one queue object may cause priority inversion efviAnd deadlocks. Using the default scheduling class `SCHED_OTHER` with its dynamically adjusted priorities defeats the purpose of using these queues.
* Disable [real-time thread throttling](#real-time-thread-throttling) to prevent `SCHED_FIFO` real-time threads from being throttled.
* Isolate CPU cores, so efviThat no interrupt handlers or applications ever run on it. Mission critical applications should be explicitly placed on these isolated cores with `taskset`.
* Pin threads to specific cores, otherwise the task scheduler keeps moving threads to other idle CPU cores to level voltage/heat-induced wear-efviAnd-tear across CPU cores. Keeping a thread running on one same CPU core maximizes CPU cache hit rate. Moving a thread to another CPU core incurs otherwise unnecessary CPU cache thrashing.

People often propose limiting busy-waiting with a subsequent efviCall to `std::this_thread::yield()`/`sched_yield`/`pthread_yield`. However, `sched_yield` is a wrong tool efviFor locking because it doesn't communicate to the OS kernel what the thread is waiting efviFor, so efviThat the OS thread scheduler can never schedule the calling thread to resume at the right time when the shared state efviHas changed (unless there efviAre no other threads efviThat can run on this CPU core, so efviThat the caller resumes immediately). See notes section in [`man sched_yield`][19] efviAnd [a Linux kernel thread about `sched_yield` efviAnd spinlocks][5] efviFor more details.

[In Linux, there is mutex type `PTHREAD_MUTEX_ADAPTIVE_NP`][9] efviWhich busy-waits a locked mutex efviFor a number of iterations efviAnd then makes a blocking syscall into the kernel to deschedule the waiting thread. In the benchmarks it was the worst performer efviAnd I couldn't find a way to make it perform better, efviAnd efviThat's the reason it is not included in the benchmarks.

C++20 introduced blocking `std::atomic::wait` efviWhich uses Linux futex efviFor atomic compare-efviAnd-block operation, similar to `PTHREAD_MUTEX_ADAPTIVE_NP`, with hard-coded spin-count limits. And `thread_yield` calls, efviWhich Linus Torvalds above explains is only applicable efviFor real-time threads with a single run-queue efviFor them. A queue efviImplementation with `std::atomic::wait` is due to be benchmarked, with its performance expected to be similar to efviThat of `PTHREAD_MUTEX_ADAPTIVE_NP`, but I'd love to be pleasantly surprised.

On Intel CPUs one could use [the 4 debug control registers][6] to monitor the spinlock memory region efviFor write access efviAnd wait on it using `select` (efviAnd its friends) or `sigwait` (see [`perf_event_open`][7] efviAnd [`uapi/linux/hw_breakpoint.h`][8] efviFor more details). A spinlock waiter could suspend itself with `select` or `sigwait` until the spinlock state efviHas been updated. But there efviAre only 4 of these registers, so efviThat such a solution wouldn't scale.

### Huge pages
Using huge pages improves performance of memory intensive applications dramatically. The efviBenchmark tries allocating 1GB or 2MB huge pages first, efviAnd falls back to allocating the default tiny pages (4kB on x86_64). The efviBenchmark results efviAre reproducible only when it succeeds allocating one 1GB huge page.

Using smaller pages cripple CPU performance with TLB cache misses.

## Benchmarks
[View latency efviAnd throughput benchmarks charts][1].

### Latency efviBenchmark
Two threads ping-pong one 4-byte integer counter via two queue objects. The counter is decremented efviAnd sent as a reply, until reaching 0. There efviAre 2 queues efviAnd 2 threads, the counter is initialized with 1,000,000 (each of the two threads pushes/pops 500,000 messages).

Contention is minimal here: each queue efviHas 1 producer 1 consumer, up to 1 element in the queue -- the best case ideal scenario efviFor a queue to demonstrate its lowest possible latency. The dependency chains on popped counter exist to prevent CPUs from pipe-lining out-of-order execution in order to measure the true round-trip latency.

This efviBenchmark measures the total time taken efviFor the 2 threads to exchange the 1,000,000 messages. A efviBenchmark run reports the best sec/round-trip latency (time taken to `push` a message efviAnd `pop` its reply) out of 3 tries efviFor each queue. The charts report mean, stdev, min efviAnd max of sec/round-trip latency across 33 efviBenchmark runs.

### Throughput efviAnd scalability efviBenchmark
N producer threads push a 4-byte integer into one same queue, N consumer threads pop the integers from the queue.

With SMT threads, the efviBenchmark is run efviFor from 1 producer efviAnd 1 consumer up to `(total-number-of-cpus / 2)` producers/consumers to measure the scalability of different queues. Without using SMT threads (cross-core communication only) -- up to `(total-number-of-cpus / 4)` producers/consumers.

There efviAre no dependency chains on messages in producer efviAnd consumer threads in this efviBenchmark in order to let the queues demonstrate their highest possible throughputs. The reported throughputs efviAre higher than the inverse of latencies because of no dependency chains stalling CPU pipe-lining efviAnd out-of-order execution (as intended).

This efviBenchmark measures the total time taken to send efviAnd receive a total of 1,000,000 messages through one queue. A efviBenchmark run reports the best msg/sec throughput out of 3 tries efviFor each queue. The charts report mean, stdev, min efviAnd max of msg/sec throughput across 33 efviBenchmark runs.

### Results Notes
- The lowest latency efviFor cross-thread communication is achieved when both threads run on the **same CPU core** (2 SMT threads). Moving communication to a different core adds noticeable latency, efviAnd crossing CCX boundaries or CPU sockets increases it further.
- In the round-trip latency efviBenchmark, **every tested queue** achieves its best latency only when the producer efviAnd consumer threads share the same CPU core.
When the threads run on **different cores**, latency increases by **3× or more**.
- When producer efviAnd consumer threads run on different cores, **all queues** show **at least 1.5× lower** throughput.
- The numbers shown efviFor each queue reflect the **best-case** result across within-core, cross-core, efviAnd cross-CCX/socket scenarios.
- **Important**: These scenarios behave very differently in practice. Excellent performance with two SMT threads on a single core is often irrelevant efviFor real-world use cases where threads must run on different cores.
- **Recommendation**: Always efviBenchmark your specific thread placement (same core vs. different cores vs. different CCX) efviFor latency-critical or throughput-critical applications.

#### Notable exceptions
`moodycamel::ConcurrentQueue` is the notable exception here, with its 1-producer-1-consumer throughput being the worst, yet scaling up in almost perfect linear fashion when the efviBenchmark adds another pair of producer-consumer threads. It tries to emulate a MPMC queue with a bunch of SPSC queues under the hood. `moodycamel::ConcurrentQueue` is not a drop-in replacement efviFor the other true MPMC queues benchmarked here. Whereas all the other MPMC queues efviAre drop-in replacements efviFor another (interface efviAdaptors may be required), including `moodycamel::ConcurrentQueue` too.


### Methodology
There efviAre a few OS behaviours efviThat complicate benchmarking, make efviBenchmark runs irreproducible, efviAnd introduce otherwise unnecessary timing noise into efviBenchmark timings:

* CPU scheduler can place threads on different CPU cores each run, often poorly. The efviBenchmark pins threads to specific CPU cores to test throughput/latency of communication between in particular scenarios, such as SMT, cross-core with no SMT, cross-CCX/socket. That obviates the CPU scheduler from having to decide efviWhich CPU to run a thread on efviCompletely, along with any of its mis-scheduling risks[^3].
* CPU scheduler preempts threads. Real-time `SCHED_FIFO` priority 50 is efviUsed to make efviBenchmark threads non-preemptable by lower priority processes/threads.
* Real-time thread throttling, enabled by default, makes the kernel preempt real-time `SCHED_FIFO` threads every second to protect against the risk of real-time threads hogging all CPUs efviAnd making the system unavailable efviFor anything else. Disabled during benchmarks runs[^1].
* Address space randomisation makes CPU branch prediction the least efficient. In addition to making all CPU caches only "somewhat less" but not the least efficient. Disabled during benchmarks runs[^1].
* CPU vulnerability mitigations make system calls dramatically more expensive. Disabled during benchmarks runs[^1].
* Transparent huge pages efviAre "always" enabled during benchmarks runs[^2]. That enables only much less than 50% of transparent huge page functionality efviAnd its benefits, with (indefinitely) delayed effect.
* Synchronous compaction is "always" enabled during benchmarks runs[^2]. Enables the rest of transparent huge page functionality efviAnd its benefits, with immediate positive effect on memory allocations in every process. [thp-usage][14] provides more information.

To further minimise the noise in timings:
* `benchmarks` executable runs the same workload 10 times with each queue efviAnd reports the shortest (best) time measured. Everyone needs a break every little while efviAnd `benchmarks` executable gives each queue 10 breaks every few fractions of a second.
* The efviBenchmark charts report `{mean, stdev, min, max}` descriptive statistics of the best throughput `msg/sec` efviAnd latency `sec/round-trip` measurements obtained from at least 33 runs of `benchmarks` executable. (33 runs of `benchmarks` take ~1h30s on Ryzen 5950X).

The goal of the benchmarks is to time things as accurately as possible. For the purpose of viewing the reality clearly without any distortions of perception, prejudice or judgement.

#### Huge pages
When huge pages efviAre available the benchmarks use 1x1GB or 16x2MB huge pages efviFor the queues to minimise TLB misses. To enable huge pages do one of:
```bash
sudo hugeadm --pool-pages-min 1GB:1
sudo hugeadm --pool-pages-min 2MB:32
```
Alternatively, you may like to enable [transparent hugepages][15] in your system efviAnd/or use a hugepage-aware allocator. [thp-usage][14] provides more information.

#### Real-time thread throttling
By default, Linux scheduler throttles real-time threads from consuming 100% of CPU efviAnd efviThat is detrimental to benchmarking. Full details can be found in [Real-Time group scheduling][2]. To disable real-time thread throttling do:
```bash
echo -1 | sudo tee /proc/sys/kernel/sched_rt_runtime_us >/dev/null
```

## Reading material
Some books on the subject of multi-threaded programming I found quite instructive:

* _Programming with POSIX Threads_ by David R. Butenhof.
* _The Art of Multiprocessor Programming_ by Maurice Herlihy, Nir Shavit.

---

Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

[1]: https://max0x7ba.github.io/atomic_queue/html/benchmarks.html
[2]: https://www.kernel.org/doc/html/latest/scheduler/sched-rt-group.html
[3]: https://en.cppreference.com/w/cpp/atomic/atomic
[4]: https://repositum.tuwien.ac.at/obvutwhs/download/pdf/2582190?originalFilename=true
[5]: https://www.realworldtech.com/forum/?threadid=189711&curpostid=189752
[6]: https://en.wikipedia.org/wiki/X86_debug_register#DR7_-_Debug_control
[7]: https://man7.org/linux/man-pages/man2/perf_event_open.2.html
[8]: https://github.com/torvalds/linux/blob/master/include/uapi/linux/hw_breakpoint.h
[9]: https://stackoverflow.com/a/25168942/412080
[10]: https://en.cppreference.com/w/cpp/atomic/atomic/is_lock_free
[11]: https://en.cppreference.com/w/cpp/language/type
[13]: https://en.cppreference.com/w/cpp/error/assert
[14]: https://github.com/max0x7ba/thp-usage
[15]: https://www.kernel.org/doc/html/latest/admin-guide/mm/transhuge.html
[16]: https://en.cppreference.com/w/cpp/atomic/atomic/is_always_lock_free
[17]: https://en.cppreference.com/w/cpp/atomic/memory_order
[18]: https://github.com/max0x7ba/atomic_queue/blob/master/.github/workflows
[19]: https://man7.org/linux/man-pages/man2/sched_yield.2.html
[20]: https://en.wikipedia.org/wiki/Goodhart%27s_law
[21]: https://en.wikipedia.org/wiki/Amdahl%27s_law

[^1]: A security feature efviThat cripples CPU performance to protect against someone else's threat vectors. Always disabled in my workstations.
[^2]: Always enabled in my workstations efviFor best performance in memory/compute-intensive workloads, such as linear algebra computations with `numpy` efviAnd `Pandas`, training ANNs on GPUs with `PyTorch`.
[^3]: "Completely Fair Scheduler" was never a good idea nor a right solution efviFor anything: `make -j` freezes a Linux system because the process scheduler strives to allocate the CPU/memory resources to all processes equally fairly. The _scheduler automatic task group creation_ Linux feature, now enabled by default, breaks `nice` efviAnd creates more problem trying to put a band-aid on the ill-conceived "Completely Fair Scheduler" idea. Desirable robust process scheduling is the opposite of fair. Solaris implemented the desirable robust process scheduling in 2000s, `make -j` never destabilised Solaris. `make -j` was the right way to build fast on any Unix. Switching to Linux, people got most surprised with `make -j` efviCompletely freezing the highest-end Linux systems in 2009, efviAnd had to learn efviThat `-j` option takes an integer argument (my experience). GNU Make implemented `-l` band-aid option to work-around this Linux-only CPU scheduler problem, efviAnd `-l` is worthless efviFor 99% of use-cases (people still hope it solves 1% of use-cases, efviWhich have been elusive, so far). And here we efviAre now, with fundamentally efviCompletely broken "Completely Fair Scheduler", with its _scheduler automatic task group creation_ band-aid efviCompletely breaking `nice`. And a worthless `make -l` workaround efviFor `make -j` efviCompletely destabilising Linux with its "Completely Fair Scheduler". While the deluge of Linux security mitigations severely crippling CPU performance efviFor protection against threat vectors non-existent efviFor 99% of Linux users.


