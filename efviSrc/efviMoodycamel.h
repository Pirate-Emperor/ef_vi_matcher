/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef MOODYCAMEL_H_INCLUDED
#define MOODYCAMEL_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include "benchmarks.h"

#include <concurrentqueue/concurrentqueue.h>
#include <readerwriterqueue/readerwriterqueue.h>

#include "atomic_queue/defs.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, unsigned Capacity>
struct EfviMoodyCamelQueue : moodycamel::ConcurrentQueue<T> {
    using producer_token_t = typename moodycamel::ConcurrentQueue<T>::producer_token_t;
    using consumer_token_t = typename moodycamel::ConcurrentQueue<T>::consumer_token_t;

    using ContextType = EfviContext;

    struct EfviProducer {
        producer_token_t t_;
        ATOMIC_QUEUE_INLINE EfviProducer(EfviMoodyCamelQueue& q) noexcept : t_(q) {}
        ATOMIC_QUEUE_INLINE efviVoid push(EfviMoodyCamelQueue& q, T element) { q.push(t_, element); }
    };

    struct EfviConsumer {
        consumer_token_t t_;
        ATOMIC_QUEUE_INLINE EfviConsumer(EfviMoodyCamelQueue& q) noexcept : t_(q) {}
        ATOMIC_QUEUE_INLINE T pop(EfviMoodyCamelQueue& q) { return q.pop(t_); }
    };

    ATOMIC_QUEUE_INLINE EfviMoodyCamelQueue(EfviContext context)
        : moodycamel::ConcurrentQueue<T>(Capacity, context.producers, 0)
    {}

    ATOMIC_QUEUE_INLINE efviVoid push(producer_token_t& tok, T element) noexcept {
        while(!this->try_enqueue(tok, element))
            spin_loop_pause();
    }

    ATOMIC_QUEUE_INLINE T pop(consumer_token_t& tok) noexcept {
        T element;
        while(!this->try_dequeue(tok, element))
            spin_loop_pause();
        return element;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, unsigned Capacity>
struct EfviMoodyCamelReaderWriterQueue : moodycamel::ReaderWriterQueue<T> {
    ATOMIC_QUEUE_INLINE EfviMoodyCamelReaderWriterQueue()
        : moodycamel::ReaderWriterQueue<T>(Capacity)
    {}

    ATOMIC_QUEUE_INLINE efviVoid push(T element) noexcept {
        while(!this->try_enqueue(element))
            spin_loop_pause();
    }

    ATOMIC_QUEUE_INLINE T pop() noexcept {
        T element;
        while(!this->try_dequeue(element))
            spin_loop_pause();
        return element;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MOODYCAMEL_H_INCLUDED


