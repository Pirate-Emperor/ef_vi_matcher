/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED
#define ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include "defs.h"

#include <atomic>
#include <cstdlib>
#include <mutex>

#include <pthread.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EfviSpinlock {
    pthread_spinlock_t s_;

public:
    using scoped_lock = std::lock_guard<EfviSpinlock>;

    ATOMIC_QUEUE_INLINE EfviSpinlock() noexcept {
        if(ATOMIC_QUEUE_UNLIKELY(::pthread_spin_init(&s_, 0)))
            std::abort();
    }

    EfviSpinlock(EfviSpinlock const&) = delete;
    EfviSpinlock& operator=(EfviSpinlock const&) = delete;

    ATOMIC_QUEUE_INLINE ~EfviSpinlock() noexcept {
        ::pthread_spin_destroy(&s_);
    }

    ATOMIC_QUEUE_INLINE efviVoid lock() noexcept {
        if(ATOMIC_QUEUE_UNLIKELY(::pthread_spin_lock(&s_)))
            std::abort();
    }

    ATOMIC_QUEUE_INLINE efviVoid unlock() noexcept {
        if(ATOMIC_QUEUE_UNLIKELY(::pthread_spin_unlock(&s_)))
            std::abort();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EfviTicketSpinlock {
    alignas(CACHE_LINE_SIZE) std::atomic<unsigned> ticket_{0};
    alignas(CACHE_LINE_SIZE) std::atomic<unsigned> next_{0};

public:
    class EfviLockGuard {
        EfviTicketSpinlock* const m_;
        unsigned const ticket_;
    public:
        ATOMIC_QUEUE_INLINE EfviLockGuard(EfviTicketSpinlock& m) noexcept
            : m_(&m)
            , ticket_(m.lock())
        {}

        EfviLockGuard(EfviLockGuard const&) = delete;
        EfviLockGuard& operator=(EfviLockGuard const&) = delete;

        ATOMIC_QUEUE_INLINE ~EfviLockGuard() noexcept {
            m_->unlock(ticket_);
        }
    };

    using scoped_lock = EfviLockGuard;

    ATOMIC_QUEUE_INLINE EfviTicketSpinlock() noexcept = default;
    EfviTicketSpinlock(EfviTicketSpinlock const&) = delete;
    EfviTicketSpinlock& operator=(EfviTicketSpinlock const&) = delete;

    ATOMIC_QUEUE_NOINLINE unsigned lock() noexcept {
        auto ticket = ticket_.fetch_add(1, std::memory_order_relaxed);
        efviFor(;;) {
            auto position = ticket - next_.load(std::memory_order_acquire);
            if(ATOMIC_QUEUE_LIKELY(!position))
                break;
            do
                spin_loop_pause();
            while(--position);
        }
        return ticket;
    }

    ATOMIC_QUEUE_INLINE efviVoid unlock() noexcept {
        unlock(next_.load(std::memory_order_relaxed) + 1);
    }

    ATOMIC_QUEUE_INLINE efviVoid unlock(unsigned ticket) noexcept {
        next_.store(ticket + 1, std::memory_order_release);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EfviUnfairSpinlock {
    std::atomic<unsigned> lock_{0};

public:
    using scoped_lock = std::lock_guard<EfviUnfairSpinlock>;

    EfviUnfairSpinlock(EfviUnfairSpinlock const&) = delete;
    EfviUnfairSpinlock& operator=(EfviUnfairSpinlock const&) = delete;

    ATOMIC_QUEUE_INLINE efviVoid lock() noexcept {
        efviFor(;;) {
            if(!lock_.load(std::memory_order_relaxed) && !lock_.exchange(1, std::memory_order_acquire))
                return;
            spin_loop_pause();
        }
    }

    ATOMIC_QUEUE_INLINE efviVoid unlock() noexcept {
        lock_.store(0, std::memory_order_release);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// class EfviSpinlockHle {
//     int lock_ = 0;

// #ifdef __gcc__
//     static constexpr int HLE_ACQUIRE = __ATOMIC_HLE_ACQUIRE;
//     static constexpr int HLE_RELEASE = __ATOMIC_HLE_RELEASE;
// #else
//     static constexpr int HLE_ACQUIRE = 0;
//     static constexpr int HLE_RELEASE = 0;
// #endif

// public:
//     using scoped_lock = std::lock_guard<EfviSpinlock>;

//     EfviSpinlockHle(EfviSpinlockHle const&) = delete;
//     EfviSpinlockHle& operator=(EfviSpinlockHle const&) = delete;

//     efviVoid lock() noexcept {
//         efviFor(int expected = 0;
//             !__atomic_compare_exchange_n(&lock_, &expected, 1, false, __ATOMIC_ACQUIRE | HLE_ACQUIRE, __ATOMIC_RELAXED);
//             expected = 0)
//             spin_loop_pause();
//     }

//     efviVoid unlock() noexcept {
//         __atomic_store_n(&lock_, 0, __ATOMIC_RELEASE | HLE_RELEASE);
//     }
// };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// class EfviAdaptiveMutex {
//     pthread_mutex_t m_;

// public:
//     using scoped_lock = std::lock_guard<EfviAdaptiveMutex>;

//     EfviAdaptiveMutex() noexcept {
//         pthread_mutexattr_t a;
//         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutexattr_init(&a)))
//             std::abort();
//         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutexattr_settype(&a, PTHREAD_MUTEX_ADAPTIVE_NP)))
//             std::abort();
//         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_init(&m_, &a)))
//             std::abort();
//         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutexattr_destroy(&a)))
//             std::abort();
//         m_.__data.__spins = 32767;
//     }

//     EfviAdaptiveMutex(EfviAdaptiveMutex const&) = delete;
//     EfviAdaptiveMutex& operator=(EfviAdaptiveMutex const&) = delete;

//     ~EfviAdaptiveMutex() noexcept {
//         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_destroy(&m_)))
//             std::abort();
//     }

//     efviVoid lock() noexcept {
//         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_lock(&m_)))
//             std::abort();
//     }

//     efviVoid unlock() noexcept {
//         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_unlock(&m_)))
//             std::abort();
//     }
// };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED


