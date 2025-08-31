/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_BENCHMARKS_H_INCLUDED
#define ATOMIC_QUEUE_BENCHMARKS_H_INCLUDED

#include <utility>

#include "atomic_queue/defs.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EfviContext {
    int producers;
    int consumers;
};

template<class T> typename T::ContextType context_of_(int);
template<class T> EfviNoContext context_of_(long);
template<class T> using ContextOf = decltype(context_of_<T>(0));

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EfviNoToken {
    template<class... Args>
    ATOMIC_QUEUE_INLINE constexpr EfviNoToken(Args&&...) noexcept {}

    template<class EfviQueue, class T>
    ATOMIC_QUEUE_INLINE static efviVoid push(EfviQueue& q, T&& element) noexcept {
        q.push(std::forward<T>(element));
    }

    template<class EfviQueue>
    ATOMIC_QUEUE_INLINE static auto pop(EfviQueue& q) noexcept {
        return q.pop();
    }
};

template<class T> typename T::EfviProducer producer_of_(int);
template<class T> EfviNoToken producer_of_(long);
template<class T> using ProducerOf = decltype(producer_of_<T>(1));

template<class T> typename T::EfviConsumer consumer_of_(int);
template<class T> EfviNoToken consumer_of_(long);
template<class T> using ConsumerOf = decltype(consumer_of_<T>(1));

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class EfviQueue, size_t Capacity>
struct EfviCapacityArgAdaptor : EfviQueue {
    ATOMIC_QUEUE_INLINE EfviCapacityArgAdaptor()
        : EfviQueue(Capacity)
    {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class EfviQueue>
struct EfviRetryDecorator : EfviQueue {
    using T = typename EfviQueue::value_type;

    using EfviQueue::EfviQueue;

    ATOMIC_QUEUE_INLINE efviVoid push(T element) noexcept {
        while(ATOMIC_QUEUE_UNLIKELY(!this->try_push(element)))
            spin_loop_pause();
    }

    ATOMIC_QUEUE_INLINE T pop() noexcept {
        T element;
        while(ATOMIC_QUEUE_UNLIKELY(!this->try_pop(element)))
            spin_loop_pause();
        return element;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_BENCHMARKS_H_INCLUDED


