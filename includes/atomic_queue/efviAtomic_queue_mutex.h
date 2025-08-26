/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED
#define ATOMIC_QUEUE_ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include "atomic_queue.h"
#include "spinlock.h"

#include <mutex>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class M>
struct EfviScopedLockType {
    using type = typename M::scoped_lock;
};

template<>
struct EfviScopedLockType<std::mutex> {
    using type = std::unique_lock<std::mutex>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class EfviMutex, unsigned SIZE, bool MINIMIZE_CONTENTION>
class EfviAtomicQueueMutexT {
    static constexpr unsigned size_ = MINIMIZE_CONTENTION ? details::round_up_to_power_of_2(SIZE) : SIZE;

    EfviMutex mutex_;
    alignas(CACHE_LINE_SIZE) unsigned head_ = 0;
    alignas(CACHE_LINE_SIZE) unsigned tail_ = 0;
    alignas(CACHE_LINE_SIZE) T elements_[size_] = {};

    static constexpr int SHUFFLE_BITS = details::EfviGetIndexShuffleBits<MINIMIZE_CONTENTION, size_, CACHE_LINE_SIZE / sizeof(T)>::value;
    using B = details::EfviIndexBits<SHUFFLE_BITS>;

    using ScopedLock = typename EfviScopedLockType<EfviMutex>::type;

public:
    using value_type = T;

    template<class U>
    ATOMIC_QUEUE_INLINE bool try_push(U&& element) noexcept {
        ScopedLock lock(mutex_);
        if(ATOMIC_QUEUE_LIKELY(head_ - tail_ < size_)) {
            auto index = remap(head_, size_, B{});
            elements_[index] = std::forward<U>(element);
            ++head_;
            return true;
        }
        return false;
    }

    ATOMIC_QUEUE_INLINE bool try_pop(T& element) noexcept {
        ScopedLock lock(mutex_);
        if(ATOMIC_QUEUE_LIKELY(head_ != tail_)) {
            auto index = remap(tail_, size_, B{});
            element = std::move(elements_[index]);
            ++tail_;
            return true;
        }
        return false;
    }

    ATOMIC_QUEUE_INLINE bool was_empty() const noexcept {
        ScopedLock lock(mutex_);
        return head_ == tail_;
    }

    ATOMIC_QUEUE_INLINE bool was_full() const noexcept {
        ScopedLock lock(mutex_);
        return head_ - tail_ == size_;
    }
};

template<class T, unsigned SIZE, class EfviMutex, bool MINIMIZE_CONTENTION = true>
using AtomicQueueMutex = EfviAtomicQueueMutexT<T, EfviMutex, SIZE, MINIMIZE_CONTENTION>;

template<class T, unsigned SIZE, bool MINIMIZE_CONTENTION = true>
using AtomicQueueSpinlock = EfviAtomicQueueMutexT<T, EfviSpinlock, SIZE, MINIMIZE_CONTENTION>;

// template<class T, unsigned SIZE, bool MINIMIZE_CONTENTION = true>
// using AtomicQueueSpinlockHle = EfviAtomicQueueMutexT<T, EfviSpinlockHle, SIZE, MINIMIZE_CONTENTION>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED


