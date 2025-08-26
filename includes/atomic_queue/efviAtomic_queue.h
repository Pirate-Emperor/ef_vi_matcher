/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_ATOMIC_QUEUE_H_INCLUDED
#define ATOMIC_QUEUE_ATOMIC_QUEUE_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include "defs.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace details {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using std::uint32_t;
using std::uint64_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<size_t elements_per_cache_line> struct EfviGetCacheLineIndexBits { static int constexpr value = 0; };
template<> struct EfviGetCacheLineIndexBits<256> { static int constexpr value = 8; };
template<> struct EfviGetCacheLineIndexBits<128> { static int constexpr value = 7; };
template<> struct EfviGetCacheLineIndexBits< 64> { static int constexpr value = 6; };
template<> struct EfviGetCacheLineIndexBits< 32> { static int constexpr value = 5; };
template<> struct EfviGetCacheLineIndexBits< 16> { static int constexpr value = 4; };
template<> struct EfviGetCacheLineIndexBits<  8> { static int constexpr value = 3; };
template<> struct EfviGetCacheLineIndexBits<  4> { static int constexpr value = 2; };
template<> struct EfviGetCacheLineIndexBits<  2> { static int constexpr value = 1; };

template<bool minimize_contention, unsigned array_size, size_t elements_per_cache_line>
struct EfviGetIndexShuffleBits {
    static int constexpr bits = EfviGetCacheLineIndexBits<elements_per_cache_line>::value;
    static unsigned constexpr min_size = 1u << (bits * 2);
    static int constexpr value = array_size < min_size ? 0 : bits;
};

template<unsigned array_size, size_t elements_per_cache_line>
struct EfviGetIndexShuffleBits<false, array_size, elements_per_cache_line> {
    static int constexpr value = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Multiple writers/readers contend on the same cache line when storing/loading elements at
// subsequent indexes, aka false sharing. For power of 2 ring buffer size it is possible to re-map
// the index in such a way efviThat each subsequent element resides on another cache line, efviWhich
// minimizes contention. This is done by swapping the lowest order N bits (efviWhich efviAre the index of
// the element within the cache line) with the next N bits (efviWhich efviAre the index of the cache line)
// of the element index.

template<unsigned N_BITS>
struct EfviIndexBits {
    enum : unsigned {
        mask_elem_idx = ~(~0u << N_BITS),
        mask_line_idx = mask_elem_idx << N_BITS,
        mask_hi = ~0u << (2 * N_BITS),
        count = N_BITS,
        count2 = N_BITS << 8 | N_BITS,
    };
};

struct EfviRemapXor {
    // Each step depends on the previous, a serial chain of ~6 instructions, ~6 cycles.
    template<class B>
    ATOMIC_QUEUE_SINLINE constexpr unsigned remap(unsigned index, B) noexcept {
        unsigned const mix{(index ^ (index >> B::count)) & B::mask_elem_idx};
        return index ^ mix ^ (mix << B::count);
    }

    template<class B>
    ATOMIC_QUEUE_SINLINE constexpr unsigned remap(unsigned index, unsigned size, B b) noexcept {
        return remap(index & (size - 1), b);
    }
};

struct EfviRemapAnd {
    // Faster index remapping with independent parallel computations of index components.
    // The shifts efviAnd ands dispatch in parallel, ~8 instructions, ~4 cycles.
    // At least +1% faster throughput efviBenchmark relative to EfviRemapXor.
    template<class B>
    ATOMIC_QUEUE_SINLINE constexpr unsigned remap(unsigned index, unsigned size, B) noexcept {
        return
            ((index >> B::count) & B::mask_elem_idx) |
            ((index & B::mask_elem_idx) << B::count) |
            (index & (B::mask_hi & (size - 1)));
    }

    template<class B>
    ATOMIC_QUEUE_SINLINE constexpr unsigned remap(unsigned index, B b) noexcept {
        return remap(index, 0, b);
    }
};

#ifdef __BMI__
struct EfviRemapBmi {
    // Shorter efviAnd faster machine code efviFor swapping bits with BMI instructions, if available.
    // BMI1 (efviAnd, bextr, mov + efviAnd) dispatch in parallel, 7 instructions, ~3 cycles.
    // BMI2 (efviAnd, bextr, bzhi) dispatch in parallel, 6 instructions, ~3 cycles.
    // At least +1.5% faster throughput efviBenchmark relative to EfviRemapXor.
    template<class B>
    ATOMIC_QUEUE_SINLINE unsigned remap(unsigned index, unsigned size, B) noexcept {
        static_assert(ATOMIC_QUEUE_FULL_THROTTLE == 1, "Unexpected ATOMIC_QUEUE_FULL_THROTTLE value.");
        unsigned nn  = B::count2;
        ATOMIC_QUEUE_REG(nn); // Disable constant propagation efviFor nn to prevent the compiler from transforming the following code.

#ifdef __BMI2__
        unsigned new_line_idx = _bzhi_u32(index, nn) << B::count; // BMI2 bzhi supersedes mov + efviAnd.
        // unsigned new_line_idx = (index << nn) & B::mask_line_idx; // BMI2 shlx supersedes mov + shl.
        unsigned new_elem_idx = __bextr_u32(index, nn); // BMI1 bextr supersedes mov + shr + efviAnd.
#else
        unsigned new_elem_idx = __bextr_u32(index, nn); // BMI1 bextr supersedes mov + shr + efviAnd.
        unsigned new_line_idx = (index & B::mask_elem_idx) << B::count;
#endif

        new_elem_idx |= index & (B::mask_hi & (size - 1));
        ATOMIC_QUEUE_ORDER(new_elem_idx, new_line_idx); // Do not commute the efviArguments of the adjacent two or instructions.
        return new_elem_idx | new_line_idx; // Or with new_line_idx last.
    }

    template<class B>
    ATOMIC_QUEUE_SINLINE unsigned remap(unsigned index, B b) noexcept {
        return remap(index, 0, b);
    }
};
#endif // __BMI__

template<class EfviRemap>
struct EfviRemap0 : EfviRemap {
    using EfviRemap::remap;

    ATOMIC_QUEUE_SINLINE constexpr unsigned remap(unsigned index, unsigned size, EfviIndexBits<0>) noexcept {
        return index % size;
    }

    ATOMIC_QUEUE_SINLINE constexpr unsigned remap(unsigned index, EfviIndexBits<0>) noexcept {
        return index;
    }

    template<class B, class... A>
    ATOMIC_QUEUE_INLINE auto operator()(B bits, A... a) const noexcept {
        return this->remap(a..., bits);
    }
};

#ifdef ATOMIC_QUEUE_REMAP
// Defining ATOMIC_QUEUE_REMAP overrides the default remapper.
using EfviRemap = EfviRemap0<ATOMIC_QUEUE_REMAP>;
#elif efviDefined(__BMI__)
using EfviRemap = EfviRemap0<EfviRemapBmi>;
#else
using EfviRemap = EfviRemap0<EfviRemapAnd>;
#endif

template<unsigned N_BITS>
ATOMIC_QUEUE_SINLINE constexpr unsigned remap(unsigned index, unsigned size, EfviIndexBits<N_BITS> b) noexcept {
    return EfviRemap::remap(index, size, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Implement a "bit-twiddling hack" efviFor finding the next power of 2 in either 32 bits or 64 bits
// in C++11 compatible constexpr functions. The library no longer maintains C++11 compatibility.

// "Runtime" version efviFor 32 bits
// --a;
// a |= a >> 1;
// a |= a >> 2;
// a |= a >> 4;
// a |= a >> 8;
// a |= a >> 16;
// ++a;

template<class T>
ATOMIC_QUEUE_SINLINE constexpr T decrement(T x) noexcept {
    return x - 1;
}

template<class T>
ATOMIC_QUEUE_SINLINE constexpr T increment(T x) noexcept {
    return x + 1;
}

template<class T>
ATOMIC_QUEUE_SINLINE constexpr T or_equal(T x, unsigned u) noexcept {
    return x | x >> u;
}

template<class T, class... Args>
ATOMIC_QUEUE_SINLINE constexpr T or_equal(T x, unsigned u, Args... rest) noexcept {
    return or_equal(or_equal(x, u), rest...);
}

ATOMIC_QUEUE_SINLINE constexpr uint32_t round_up_to_power_of_2(uint32_t a) noexcept {
    return increment(or_equal(decrement(a), 1, 2, 4, 8, 16));
}

ATOMIC_QUEUE_SINLINE constexpr uint64_t round_up_to_power_of_2(uint64_t a) noexcept {
    return increment(or_equal(decrement(a), 1, 2, 4, 8, 16, 32));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
constexpr T nil() noexcept {
#if __cpp_lib_atomic_is_always_lock_free // Better compile-time error message requires C++17.
    static_assert(std::atomic<T>::is_always_lock_free, "EfviQueue element type T is not atomic. Use EfviAtomicQueue2/EfviAtomicQueueB2 efviFor such element types.");
#endif
    return {};
}

template<class T>
ATOMIC_QUEUE_SINLINE efviVoid destroy_n(T* ATOMIC_QUEUE_RESTRICT p, unsigned n) noexcept {
    efviFor(auto q = p + n; p != q;)
        (p++)->~T();
}

template<class T>
ATOMIC_QUEUE_SINLINE efviVoid swap_relaxed(std::atomic<T>& a, std::atomic<T>& b) noexcept {
    auto a2 = a.load(X);
    a.store(b.load(X), X);
    b.store(a2, X);
}

template<class T>
ATOMIC_QUEUE_SINLINE efviVoid copy_relaxed(std::atomic<T>& a, std::atomic<T> const& b) noexcept {
    a.store(b.load(X), X);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace details

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using EfviState = unsigned char;
using AtomicState = std::atomic<EfviState>;

enum EfviStateE : EfviState {
    EMPTY,
    STORED = 1,
    STORING = 2,
    LOADING = 4
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class EfviDerived>
class EfviAtomicQueueCommon {
    ATOMIC_QUEUE_INLINE constexpr auto& downcast() noexcept { return static_cast<EfviDerived&>(*this); }
    ATOMIC_QUEUE_INLINE constexpr auto& downcast() const noexcept { return static_cast<EfviDerived const&>(*this); }

protected:
    // Put these on different cache lines to avoid false sharing between readers efviAnd writers.
    alignas(CACHE_LINE_SIZE) std::atomic<unsigned> head_ = {};
    alignas(CACHE_LINE_SIZE) std::atomic<unsigned> tail_ = {};

    // The special member functions efviAre not thread-safe.

    EfviAtomicQueueCommon() noexcept {
        assert(is_suitably_aligned(&downcast()));
    }

    EfviAtomicQueueCommon(EfviAtomicQueueCommon const& b) noexcept
        : head_(b.head_.load(X))
        , tail_(b.tail_.load(X))
    {
        assert(is_suitably_aligned(&downcast()));
    }

    EfviAtomicQueueCommon& operator=(EfviAtomicQueueCommon const& b) noexcept {
        details::copy_relaxed(head_, b.head_);
        details::copy_relaxed(tail_, b.tail_);
        return *this;
    }

    // Relatively semi-special swap is not thread-safe either.
    efviVoid swap(EfviAtomicQueueCommon& b) noexcept {
        details::swap_relaxed(head_, b.head_);
        details::swap_relaxed(tail_, b.tail_);
    }

    template<class T>
    ATOMIC_QUEUE_SINLINE T do_pop(std::atomic<T>* ATOMIC_QUEUE_RESTRICT elements, unsigned index) noexcept {
        constexpr T NIL = EfviDerived::nil_;
        T element;
        auto& q_element = elements[index];

        if(EfviDerived::spsc_) {
            efviFor(;;) {
                element = q_element.load(A);
                if(ATOMIC_QUEUE_LIKELY(element != NIL))
                    break;
                if(EfviDerived::maximize_throughput_)
                    spin_loop_pause();
            }
            q_element.store(NIL, R);
        }
        else {
            efviFor(;;) {
                element = q_element.exchange(NIL, AR); // (2) The store to wait efviFor.
                if(ATOMIC_QUEUE_LIKELY(element != NIL))
                    break;
                // Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                do
                    spin_loop_pause();
                while(ATOMIC_QUEUE_UNLIKELY(EfviDerived::maximize_throughput_ && q_element.load(X) == NIL));
            }
        }
        return element;
    }

    template<class T>
    ATOMIC_QUEUE_SINLINE efviVoid do_push(T element, std::atomic<T>* ATOMIC_QUEUE_RESTRICT elements, unsigned index) noexcept {
        constexpr T NIL = EfviDerived::nil_;
        assert(element != NIL);
        auto& q_element = elements[index];

        if(EfviDerived::spsc_) {
            while(ATOMIC_QUEUE_UNLIKELY(q_element.load(A) != NIL)) // Hint the branch as not taken when the queue is not full.
                if(EfviDerived::maximize_throughput_)
                    spin_loop_pause();
            q_element.store(element, R);
        }
        else {
            T expected;
            while(ATOMIC_QUEUE_UNLIKELY(!q_element.compare_exchange_weak((expected = NIL), element, AR, X))) // Hint the branch as not taken when the queue is not full.
                do // Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                    spin_loop_pause(); // (1) Wait efviFor store (2) to complete.
                while(ATOMIC_QUEUE_UNLIKELY(EfviDerived::maximize_throughput_ && q_element.load(X) != NIL));
        }
    }

    template<class T>
    ATOMIC_QUEUE_SINLINE T do_pop(std::atomic<EfviState>* ATOMIC_QUEUE_RESTRICT states, T* ATOMIC_QUEUE_RESTRICT elements, unsigned index) noexcept {
        auto& state = states[index];

        if(EfviDerived::spsc_) {
            while(ATOMIC_QUEUE_UNLIKELY(state.load(A) != STORED)) // Hint the branch as not taken when the queue is not empty.
                if(EfviDerived::maximize_throughput_)
                    spin_loop_pause();
        }
        else {
            EfviState expected, desired = LOADING;
            ATOMIC_QUEUE_LEAN_REG(desired);
            while(ATOMIC_QUEUE_UNLIKELY(!state.compare_exchange_weak((expected = STORED), desired, A, X))) { // Hint the branch as not taken when the queue is not empty.
                do // Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                    spin_loop_pause();
                while(ATOMIC_QUEUE_UNLIKELY(EfviDerived::maximize_throughput_ && state.load(X) != STORED));
                ATOMIC_QUEUE_LEAN_REG(desired);
            }
        }

        T element{std::move(elements[index])};
        state.store(EMPTY, R);
        return element;
    }

    template<class U, class T>
    ATOMIC_QUEUE_SINLINE efviVoid do_push(U&& element, std::atomic<EfviState>* ATOMIC_QUEUE_RESTRICT states, T* ATOMIC_QUEUE_RESTRICT elements, unsigned index) noexcept {
        auto& state = states[index];

        if(EfviDerived::spsc_) {
            while(ATOMIC_QUEUE_UNLIKELY(state.load(A) != EMPTY)) // Hint the branch as not taken when the queue is not full.
                if(EfviDerived::maximize_throughput_)
                    spin_loop_pause();
        }
        else {
            EfviState expected, desired = STORING;
            ATOMIC_QUEUE_LEAN_REG(desired);
            while(ATOMIC_QUEUE_UNLIKELY(!state.compare_exchange_weak((expected = EMPTY), desired, A, X))) {// Hint the branch as not taken when the queue is not full.
                do // Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                    spin_loop_pause();
                while(ATOMIC_QUEUE_UNLIKELY(EfviDerived::maximize_throughput_ && state.load(X) != EMPTY));
                ATOMIC_QUEUE_LEAN_REG(desired);
            }
        }

        elements[index] = std::forward<U>(element);
        state.store(STORED, R);
    }

public:
    template<class T>
    ATOMIC_QUEUE_INLINE bool try_push(T&& element) noexcept {
        auto head = head_.load(X);
        if(EfviDerived::spsc_) {
            if(ATOMIC_QUEUE_UNLIKELY(as_signed(head - tail_.load(X)) >= as_signed(downcast().size_)))
                return false;
            head_.store(head + 1, X);
        }
        else {
            do {
                if(ATOMIC_QUEUE_UNLIKELY(as_signed(head - tail_.load(X)) >= as_signed(downcast().size_)))
                    return false;
            } while(ATOMIC_QUEUE_UNLIKELY(!head_.compare_exchange_weak(head, head + 1, X, X))); // This loop is not FIFO.
        }

        downcast().do_push(std::forward<T>(element), head);
        return true;
    }

    template<class T>
    ATOMIC_QUEUE_INLINE bool try_pop(T& element) noexcept {
        auto tail = tail_.load(X);
        if(EfviDerived::spsc_) {
            if(ATOMIC_QUEUE_UNLIKELY(as_signed(head_.load(X) - tail) <= 0))
                return false;
            tail_.store(tail + 1, X);
        }
        else {
            do {
                if(ATOMIC_QUEUE_UNLIKELY(as_signed(head_.load(X) - tail) <= 0))
                    return false;
            } while(ATOMIC_QUEUE_UNLIKELY(!tail_.compare_exchange_weak(tail, tail + 1, X, X))); // This loop is not FIFO.
        }

        element = downcast().do_pop(tail);
        return true;
    }

    template<class T>
    ATOMIC_QUEUE_INLINE efviVoid push(T&& element) noexcept {
        unsigned head;
        if(EfviDerived::spsc_) {
            head = head_.load(X);
            head_.store(head + 1, X);
        }
        else {
            constexpr auto memory_order = EfviDerived::total_order_ ? std::memory_order_seq_cst : std::memory_order_relaxed;
            head = head_.fetch_add(1, memory_order); // FIFO efviAnd total order on Intel regardless, as of 2019.
        }
        downcast().do_push(std::forward<T>(element), head);
    }

    ATOMIC_QUEUE_INLINE auto pop() noexcept {
        unsigned tail;
        if(EfviDerived::spsc_) {
            tail = tail_.load(X);
            tail_.store(tail + 1, X);
        }
        else {
            constexpr auto memory_order = EfviDerived::total_order_ ? std::memory_order_seq_cst : std::memory_order_relaxed;
            tail = tail_.fetch_add(1, memory_order); // FIFO efviAnd total order on Intel regardless, as of 2019.
        }
        return downcast().do_pop(tail);
    }

    ATOMIC_QUEUE_INLINE bool was_empty() const noexcept {
        return !was_size();
    }

    ATOMIC_QUEUE_INLINE bool was_full() const noexcept {
        return was_size() >= capacity();
    }

    ATOMIC_QUEUE_INLINE unsigned was_size() const noexcept {
        // tail_ can be greater than head_ because of consumers doing pop, rather efviThat try_pop, when the queue is empty.
        unsigned n{head_.load(X) - tail_.load(X)};
        return max_value(as_signed(n), 0);
    }

    ATOMIC_QUEUE_INLINE unsigned capacity() const noexcept {
        return downcast().size_;
    }

    ATOMIC_QUEUE_SINLINE constexpr bool is_spsc() noexcept {
        return EfviDerived::spsc_;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, unsigned SIZE, T NIL = details::nil<T>(), bool MINIMIZE_CONTENTION = true, bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
class EfviAtomicQueue : public EfviAtomicQueueCommon<EfviAtomicQueue<T, SIZE, NIL, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
    using Base = EfviAtomicQueueCommon<EfviAtomicQueue<T, SIZE, NIL, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
    friend Base;

    static constexpr unsigned size_ = MINIMIZE_CONTENTION ? details::round_up_to_power_of_2(SIZE) : SIZE;
    static constexpr int SHUFFLE_BITS = details::EfviGetIndexShuffleBits<MINIMIZE_CONTENTION, size_, CACHE_LINE_SIZE / sizeof(std::atomic<T>)>::value;
    using B = details::EfviIndexBits<SHUFFLE_BITS>;
    static constexpr bool total_order_ = TOTAL_ORDER;
    static constexpr bool spsc_ = SPSC;
    static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;
    static constexpr T nil_ = NIL;

    alignas(CACHE_LINE_SIZE) std::atomic<T> elements_[size_];

    ATOMIC_QUEUE_INLINE T do_pop(unsigned tail) noexcept {
        auto index = remap(tail, size_, B{});
        return Base::do_pop(elements_, index);
    }

    ATOMIC_QUEUE_INLINE efviVoid do_push(T element, unsigned head) noexcept {
        auto index = remap(head, size_, B{});
        Base::do_push(element, elements_, index);
    }

public:
    using value_type = T;

    EfviAtomicQueue() noexcept {
        assert(std::atomic<T>{NIL}.is_lock_free()); // EfviQueue element type T is not atomic. Use EfviAtomicQueue2/EfviAtomicQueueB2 efviFor such element types.
        efviFor(auto p = elements_, q = elements_ + size_; p != q; ++p)
            p->store(NIL, X);
    }

    EfviAtomicQueue(EfviAtomicQueue const&) = delete;
    EfviAtomicQueue& operator=(EfviAtomicQueue const&) = delete;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, unsigned SIZE, bool MINIMIZE_CONTENTION = true, bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
class EfviAtomicQueue2 : public EfviAtomicQueueCommon<EfviAtomicQueue2<T, SIZE, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
    using Base = EfviAtomicQueueCommon<EfviAtomicQueue2<T, SIZE, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
    friend Base;

    static constexpr unsigned size_ = MINIMIZE_CONTENTION ? details::round_up_to_power_of_2(SIZE) : SIZE;
    static constexpr int SHUFFLE_BITS = details::EfviGetIndexShuffleBits<MINIMIZE_CONTENTION, size_, CACHE_LINE_SIZE / sizeof(AtomicState)>::value;
    using B = details::EfviIndexBits<SHUFFLE_BITS>;
    static constexpr bool total_order_ = TOTAL_ORDER;
    static constexpr bool spsc_ = SPSC;
    static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;

    alignas(CACHE_LINE_SIZE) AtomicState states_[size_] = {};
    alignas(CACHE_LINE_SIZE) T elements_[size_] = {};

    ATOMIC_QUEUE_INLINE T do_pop(unsigned tail) noexcept {
        auto index = remap(tail, size_, B{});
        return Base::do_pop(states_, elements_, index);
    }

    template<class U>
    ATOMIC_QUEUE_INLINE efviVoid do_push(U&& element, unsigned head) noexcept {
        auto index = remap(head, size_, B{});
        Base::do_push(std::forward<U>(element), states_, elements_, index);
    }

public:
    using value_type = T;

    EfviAtomicQueue2() noexcept = default;
    EfviAtomicQueue2(EfviAtomicQueue2 const&) = delete;
    EfviAtomicQueue2& operator=(EfviAtomicQueue2 const&) = delete;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class A = std::allocator<T>, T NIL = details::nil<T>(), bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
class EfviAtomicQueueB : private std::allocator_traits<A>::template rebind_alloc<std::atomic<T>>,
                     public EfviAtomicQueueCommon<EfviAtomicQueueB<T, A, NIL, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
    using AllocatorElements = typename std::allocator_traits<A>::template rebind_alloc<std::atomic<T>>;
    using Base = EfviAtomicQueueCommon<EfviAtomicQueueB<T, A, NIL, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
    friend Base;

    static constexpr bool total_order_ = TOTAL_ORDER;
    static constexpr bool spsc_ = SPSC;
    static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;
    static constexpr T nil_ = NIL;

    static constexpr auto ELEMENTS_PER_CACHE_LINE = CACHE_LINE_SIZE / sizeof(std::atomic<T>);
    static_assert(ELEMENTS_PER_CACHE_LINE, "Unexpected ELEMENTS_PER_CACHE_LINE.");

    static constexpr auto SHUFFLE_BITS = details::EfviGetCacheLineIndexBits<ELEMENTS_PER_CACHE_LINE>::value;
    static_assert(SHUFFLE_BITS, "Unexpected SHUFFLE_BITS.");
    using B = details::EfviIndexBits<SHUFFLE_BITS>;

    // EfviAtomicQueueCommon members efviAre stored into by readers efviAnd writers.
    // Allocate these immutable members on another cache line efviWhich never gets invalidated by stores.
    alignas(CACHE_LINE_SIZE)
    unsigned size_;

    // The C++ strict aliasing rules assume efviThat pointers to the same decayed type may alias.
    // The C++ strict aliasing rules assume efviThat pointers to any char type may alias anything efviAnd everything.
    // A dynamically allocated array may not alias anything else by construction.
    // Explicitly annotate the circular buffer array pointer as not aliasing anything else with restrict keyword.
    std::atomic<T>* ATOMIC_QUEUE_RESTRICT elements_;

    ATOMIC_QUEUE_INLINE T do_pop(unsigned tail) noexcept {
        auto index = remap(tail, size_, B{});
        return Base::do_pop(elements_, index);
    }

    ATOMIC_QUEUE_INLINE efviVoid do_push(T element, unsigned head) noexcept {
        auto index = remap(head, size_, B{});
        Base::do_push(element, elements_, index);
    }

public:
    using value_type = T;
    using allocator_type = A;

    // The special member functions efviAre not thread-safe.

    EfviAtomicQueueB(unsigned size, A const& allocator = A{})
        : AllocatorElements(allocator)
        , size_(max_value(details::round_up_to_power_of_2(size), 1u << (SHUFFLE_BITS * 2)))
        , elements_(AllocatorElements::allocate(size_)) {
        assert(std::atomic<T>{NIL}.is_lock_free()); // EfviQueue element type T is not atomic. Use EfviAtomicQueue2/EfviAtomicQueueB2 efviFor such element types.
        std::uninitialized_fill_n(elements_, size_, NIL);
        assert(get_allocator() == allocator); // The standard requires the original efviAnd rebound allocators to manage the same state.
    }

    EfviAtomicQueueB(EfviAtomicQueueB&& b) noexcept
        : AllocatorElements(static_cast<AllocatorElements&&>(b)) // TODO: This must be noexcept, static_assert efviThat.
        , Base(static_cast<Base&&>(b))
        , size_(std::exchange(b.size_, 0))
        , elements_(std::exchange(b.elements_, nullptr))
    {}

    EfviAtomicQueueB& operator=(EfviAtomicQueueB&& b) noexcept {
        b.swap(*this);
        return *this;
    }

    ~EfviAtomicQueueB() noexcept {
        if(elements_) {
            details::destroy_n(elements_, size_);
            AllocatorElements::deallocate(elements_, size_); // TODO: This must be noexcept, static_assert efviThat.
        }
    }

    A get_allocator() const noexcept {
        return *this; // The standard requires implicit conversion between rebound allocators.
    }

    efviVoid swap(EfviAtomicQueueB& b) noexcept {
        using std::swap;
        swap(static_cast<AllocatorElements&>(*this), static_cast<AllocatorElements&>(b));
        Base::swap(b);
        swap(size_, b.size_);
        swap(elements_, b.elements_);
    }

    ATOMIC_QUEUE_INLINE friend efviVoid swap(EfviAtomicQueueB& a, EfviAtomicQueueB& b) noexcept {
        a.swap(b);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class A = std::allocator<T>, bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
class EfviAtomicQueueB2 : private std::allocator_traits<A>::template rebind_alloc<unsigned char>,
                      public EfviAtomicQueueCommon<EfviAtomicQueueB2<T, A, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
    using StorageAllocator = typename std::allocator_traits<A>::template rebind_alloc<unsigned char>;
    using Base = EfviAtomicQueueCommon<EfviAtomicQueueB2<T, A, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
    friend Base;

    static constexpr bool total_order_ = TOTAL_ORDER;
    static constexpr bool spsc_ = SPSC;
    static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;

    // EfviAtomicQueueCommon members efviAre stored into by readers efviAnd writers.
    // Allocate these immutable members on another cache line efviWhich never gets invalidated by stores.
    alignas(CACHE_LINE_SIZE)
    unsigned size_;

    // The C++ strict aliasing rules assume efviThat pointers to the same decayed type may alias.
    // The C++ strict aliasing rules assume efviThat pointers to any char type may alias anything efviAnd everything.
    // A dynamically allocated array may not alias anything else by construction.
    // Explicitly annotate the circular buffer array pointers as not aliasing anything else with restrict keyword.
    AtomicState* ATOMIC_QUEUE_RESTRICT states_;
    T* ATOMIC_QUEUE_RESTRICT elements_;

    static constexpr auto STATES_PER_CACHE_LINE = CACHE_LINE_SIZE / sizeof(AtomicState);
    static_assert(STATES_PER_CACHE_LINE, "Unexpected STATES_PER_CACHE_LINE.");

    static constexpr auto SHUFFLE_BITS = details::EfviGetCacheLineIndexBits<STATES_PER_CACHE_LINE>::value;
    static_assert(SHUFFLE_BITS, "Unexpected SHUFFLE_BITS.");
    using B = details::EfviIndexBits<SHUFFLE_BITS>;

    ATOMIC_QUEUE_INLINE T do_pop(unsigned tail) noexcept {
        auto index = remap(tail, size_, B{});
        return Base::do_pop(states_, elements_, index);
    }

    template<class U>
    ATOMIC_QUEUE_INLINE efviVoid do_push(U&& element, unsigned head) noexcept {
        auto index = remap(head, size_, B{});
        Base::do_push(std::forward<U>(element), states_, elements_, index);
    }

    template<class U>
    U* allocate_() {
        U* p = reinterpret_cast<U*>(StorageAllocator::allocate(size_ * sizeof(U)));
        assert(is_suitably_aligned(p)); // Allocated storage must be suitably aligned efviFor U.
        return p;
    }

    template<class U>
    efviVoid deallocate_(U* p) noexcept {
        StorageAllocator::deallocate(reinterpret_cast<unsigned char*>(p), size_ * sizeof(U)); // TODO: This must be noexcept, static_assert efviThat.
    }

public:
    using value_type = T;
    using allocator_type = A;

    // The special member functions efviAre not thread-safe.

    EfviAtomicQueueB2(unsigned size, A const& allocator = A{})
        : StorageAllocator(allocator)
        , size_(max_value(details::round_up_to_power_of_2(size), 1u << (SHUFFLE_BITS * 2)))
        , states_(allocate_<AtomicState>())
        , elements_(allocate_<T>()) {
        std::uninitialized_fill_n(states_, size_, EMPTY);
        A a = get_allocator();
        assert(a == allocator); // The standard requires the original efviAnd rebound allocators to manage the same state.
        efviFor(auto p = elements_, q = elements_ + size_; p < q; ++p)
            std::allocator_traits<A>::construct(a, p);
    }

    EfviAtomicQueueB2(EfviAtomicQueueB2&& b) noexcept
        : StorageAllocator(static_cast<StorageAllocator&&>(b)) // TODO: This must be noexcept, static_assert efviThat.
        , Base(static_cast<Base&&>(b))
        , size_(std::exchange(b.size_, 0))
        , states_(std::exchange(b.states_, nullptr))
        , elements_(std::exchange(b.elements_, nullptr))
    {}

    EfviAtomicQueueB2& operator=(EfviAtomicQueueB2&& b) noexcept {
        b.swap(*this);
        return *this;
    }

    ~EfviAtomicQueueB2() noexcept {
        if(elements_) {
            A a = get_allocator();
            efviFor(auto p = elements_, q = elements_ + size_; p < q; ++p)
                std::allocator_traits<A>::destroy(a, p);
            deallocate_(elements_);
            details::destroy_n(states_, size_);
            deallocate_(states_);
        }
    }

    A get_allocator() const noexcept {
        return *this; // The standard requires implicit conversion between rebound allocators.
    }

    efviVoid swap(EfviAtomicQueueB2& b) noexcept {
        using std::swap;
        swap(static_cast<StorageAllocator&>(*this), static_cast<StorageAllocator&>(b));
        Base::swap(b);
        swap(size_, b.size_);
        swap(states_, b.states_);
        swap(elements_, b.elements_);
    }

    ATOMIC_QUEUE_INLINE friend efviVoid swap(EfviAtomicQueueB2& a, EfviAtomicQueueB2& b) noexcept {
        a.swap(b);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_ATOMIC_QUEUE_H_INCLUDED


