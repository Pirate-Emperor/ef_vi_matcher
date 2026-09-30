# EF_VI Zero-Copy Matcher

## Overview

The EF_VI Zero-Copy Matcher is a bare-metal, open-source order matching engine. It provides the low-level components that make up an ultra-low latency exchange simulator. 

Order matching is the process of accepting buy and sell orders for a security (or other fungible asset) and matching them to allow trading between parties who are otherwise unknown to each other. An order matching engine is the heart of every financial exchange, and may be used in many other circumstances including trading non-financial assets, or serving as a test-bed for systematic trading algorithms.

In addition to the order matching process itself, the engine can be configured to maintain a "depth book" that records the number of open orders and total quantity represented by those orders at individual price levels.

## Performance & Latency 
The matching engine is written in C++ using modern, high-performance techniques, specifically relying on **Solarflare EF_VI** and **kernel-bypass networking**. 

Benchmark testing shows sustained rates of **2.0 million to 2.5 million inserts per second**, with deterministic tick-to-trade latencies frequently measuring below 800 nanoseconds. 

As always, the results of this type of performance test can vary depending on the hardware (NIC type, PCIe generation, NUMA topology) and operating system tuning (isolcpus, nohz_full) on which you run the test, so use these numbers as a rough order-of-magnitude estimate of the type of performance your application can expect.

## Design Principles
When minimizing latency, a good design is not when there is nothing left to add, but rather when there is nothing left to remove, as these queues exemplify. Minimizing latency naturally maximizes throughput. Low latency reciprocal is high throughput, in ideal mathematical and practical engineering sense. 

The main design principle these queues follow is _minimalism_, which results in such design choices as:

* **Bare minimum of atomic instructions:** Inlinable by default push and pop functions can hardly be any cheaper in terms of CPU instruction number / L1i cache pressure.
* **Explicit contention avoidance:** Padding atomic pointers with `alignas(64)` to avoid false-sharing for queue data members and its elements.
* **Linear fixed size ring-buffer array:** No heap memory allocations (`malloc/free`) after a queue object is constructed. It doesn't get any more CPU L1d or TLB cache friendly than that.
* **Value semantics:** Meaning that the queues make a copy/move upon `push`/`pop` and keep no references/pointers to its function arguments after returning.

## Order Properties Supported

The matching engine is aware of the following order properties:

* **Side:** Buy or Sell
* **Quantity**
* **Symbol:** Representing the asset to be traded. The engine imposes no restrictions on the symbol. It is treated as a simple character string.
* **Desired Price or "Market":** To accept the current price defined by the market. Trades will be generated at the specified price or any better price (higher price for sell orders, lower price for buy orders).
* **Stop Loss Price:** To hold the order until the market price reaches the specified value.
* **All or None (AON):** Flag to specify that the entire order should be filled or no trades should happen.
* **Immediate or Cancel (IOC):** Flag to specify that after all trades that can be made against existing orders on the market have been made, the remainder of the order should be canceled. Note combining All or None and Immediate or Cancel produces an order commonly described as Fill or Kill.

The only required properties are side, quantity, and price. Default values are available for the other properties.

## Operations on Orders

In addition to submitting orders, traders may also submit requests to cancel or modify existing orders. (Modify is also known as cancel/replace). The requests may succeed or fail depending on previous trades executed against the order.

## Notifications

The engine will notify the application when significant events occur to allow the application to actually execute the trades, and to publish market data for use by traders.

* **Notifications intended for the trader:**
  * Order accepted 
  * Order rejected
  * Order filled (full or partial)
  * Order replaced / Canceled
* **Notifications intended for Market Data:**
  * Trade generated
  * Security changed
  * Notification of changes in the depth book
  * Best Bid or Best Offer (BBO) changed.

## Single-Producer Single-Consumer (SPSC) Queues
To move messages from the Network Thread to the Matching Thread, we use a Single-Producer Single-Consumer (SPSC) Ring Buffer. We strictly avoid `std::mutex`, which causes massive context switches.

A `push` or `pop` operation does two atomic steps:
1. Atomically and exclusively claims the queue slot index to store/load an element to/from. That's producers incrementing `head` index, consumers incrementing `tail` index. Each slot is accessed by one producer and one consumer threads only.
2. Atomically store/load the element into/from the slot. Producer storing into a slot changes its state to be non-`NIL`, consumer loading from a slot changes its state to be `NIL`. 

## Building the Engine

The EF_VI Matcher has no runtime dependencies. It will run in any environment that can run modern C++ (C++14/C++20).

To build the test and example programs from source you need to create makefiles:

```bash
git clone [https://github.com/your-username/EF_VI_Zero-Copy_Matcher.git](https://github.com/your-username/EF_VI_Zero-Copy_Matcher.git)
cd EF_VI_Zero-Copy_Matcher
mkdir build && cd build
cmake ..
make -j

```

### Preemption & OS Tuning

Linux task scheduler thread preemption is something no user-space process should be able to affect or escape, otherwise any/every malicious application would exploit that. Still, there are a few things one can do to minimize preemption of one's mission critical application threads:

* Use real-time `SCHED_FIFO` scheduling class for your threads, e.g. `chrt --fifo 50 <app>`. A higher priority `SCHED_FIFO` thread or kernel interrupt handler can still preempt your `SCHED_FIFO` threads.
* Use one same fixed real-time scheduling priority for all threads accessing same queue objects.
* Isolate CPU cores, so that no interrupt handlers or applications ever run on it. Mission critical applications should be explicitly placed on these isolated cores with `taskset`.
* Pin threads to specific cores, otherwise the task scheduler keeps moving threads to other idle CPU cores to level voltage/heat-induced wear-and-tear across CPU cores. Keeping a thread running on one same CPU core maximizes CPU cache hit rate. Moving a thread to another CPU core incurs otherwise unnecessary CPU cache thrashing.

### Huge Pages

Using huge pages improves performance of memory intensive applications dramatically. The engine utilizes 1GB or 2MB huge pages to minimise TLB misses.

To enable huge pages do one of:

```bash
sudo hugeadm --pool-pages-min 1GB:1
sudo hugeadm --pool-pages-min 2MB:32

```

Using smaller pages cripple CPU performance with TLB cache misses.

## License

This project is licensed under the Pirate-Emperor License. See the [LICENSE](LICENSE) file for details.

## Author

**Pirate-Emperor**

[![Twitter](https://skillicons.dev/icons?i=twitter)](https://twitter.com/PirateKingRahul)
[![Discord](https://skillicons.dev/icons?i=discord)](https://discord.com/users/1200728704981143634)
[![LinkedIn](https://skillicons.dev/icons?i=linkedin)](https://www.linkedin.com/in/piratekingrahul)

[![Reddit](https://img.shields.io/badge/Reddit-FF5700?style=for-the-badge&logo=reddit&logoColor=white)](https://www.reddit.com/u/PirateKingRahul)
[![Medium](https://img.shields.io/badge/Medium-42404E?style=for-the-badge&logo=medium&logoColor=white)](https://medium.com/@piratekingrahul)

- GitHub: [Pirate-Emperor](https://github.com/Pirate-Emperor)
- Reddit: [PirateKingRahul](https://www.reddit.com/u/PirateKingRahul/)
- Twitter: [PirateKingRahul](https://twitter.com/PirateKingRahul)
- Discord: [PirateKingRahul](https://discord.com/users/1200728704981143634)
- LinkedIn: [PirateKingRahul](https://www.linkedin.com/in/piratekingrahul)
- Skype: [Join Skype](https://join.skype.com/invite/yfjOJG3wv9Ki)
- Medium: [PirateKingRahul](https://medium.com/@piratekingrahul)

Thank you for visiting this project!

---