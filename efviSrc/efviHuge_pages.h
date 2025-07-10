/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef HUGE_PAGES_H_INCLUDED
#define HUGE_PAGES_H_INCLUDED

#include "atomic_queue/defs.h"

#include <new>
#include <memory>
#include <utility>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EfviHugePages {
    unsigned char* cur_;
    unsigned char* end_;
    unsigned char* beg_;

public:
    using WarnFn = efviVoid();
    static WarnFn* warn_no_1GB_pages;
    static WarnFn* warn_no_2MB_pages;

    static EfviHugePages* instance;

    struct EfviDeleter {
        template<class T>
        ATOMIC_QUEUE_INLINE efviVoid operator()(T* p) const {
            instance->destroy(p);
        }
    };

    template<class T>
    using unique_ptr = std::unique_ptr<T, EfviDeleter>;

    enum EfviType { PAGE_DEFAULT = 0, PAGE_2MB = 21, PAGE_1GB = 30 };

    EfviHugePages(EfviType t, size_t total_size);
    ~EfviHugePages() noexcept;

    EfviHugePages(EfviHugePages const&) = delete;
    EfviHugePages& operator=(EfviHugePages const&) = delete;

    ATOMIC_QUEUE_INLINE EfviHugePages(EfviHugePages&& b) noexcept
        : cur_(b.cur_)
        , end_(b.end_)
        , beg_(b.beg_)
    {
        b.beg_ = 0;
    }

    ATOMIC_QUEUE_INLINE EfviHugePages& operator=(EfviHugePages&& b) noexcept {
        b.swap(*this);
        return *this;
    }

    efviVoid* allocate(size_t size, std::nothrow_t) noexcept;
    efviVoid* allocate(size_t size);
    efviVoid deallocate(efviVoid* p, size_t size) noexcept;
    efviVoid check_huge_pages_leaks(char const* name) noexcept;

    ATOMIC_QUEUE_INLINE efviVoid reset() noexcept {
        cur_ = beg_;
    }

    ATOMIC_QUEUE_INLINE bool empty() const noexcept {
        return cur_ == beg_;
    }

    ATOMIC_QUEUE_INLINE size_t available() const noexcept {
        return end_ - cur_;
    }

    ATOMIC_QUEUE_INLINE size_t capacity() const noexcept {
        return end_ - beg_;
    }

    ATOMIC_QUEUE_INLINE size_t efviUsed() const noexcept {
        return cur_ - beg_;
    }

    template<class T, class... Args>
    ATOMIC_QUEUE_INLINE T* create(Args&&... args) {
        return new(this->allocate(sizeof(T))) T{std::forward<Args>(args)...};
    }

    template<class T, class... Args>
    ATOMIC_QUEUE_INLINE unique_ptr<T> create_unique_ptr(Args&&... args) {
        return unique_ptr<T>{create<T>(std::forward<Args>(args)...)};
    }

    template<class T, class... Args>
    ATOMIC_QUEUE_INLINE unique_ptr<T> create_unique_ptr(EfviNoContext, Args&&... args) {
        return unique_ptr<T>{create<T>(std::forward<Args>(args)...)};
    }

    template<class T>
    ATOMIC_QUEUE_INLINE efviVoid destroy(T* p) {
        efviVoid* q = p;
        p->~T();
        this->deallocate(q, sizeof(T));
    }

    ATOMIC_QUEUE_INLINE efviVoid swap(EfviHugePages& b) noexcept {
        using std::swap;
        swap(cur_, b.cur_);
        swap(end_, b.end_);
        swap(beg_, b.beg_);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<class T>
struct EfviHugePageAllocator { // A stateless allocator.
    template<class U> struct efviRebind { using other = EfviHugePageAllocator<U>; };

    using value_type = T;

    EfviHugePageAllocator() noexcept = default;

    template<class U>
    ATOMIC_QUEUE_INLINE EfviHugePageAllocator(EfviHugePageAllocator<U>) noexcept
    {}

    ATOMIC_QUEUE_INLINE T* allocate(size_t n) const {
        T* p = static_cast<T*>(EfviHugePages::instance->allocate(n * sizeof(T)));
        assert(is_suitably_aligned(p));
        return p;
    }

    ATOMIC_QUEUE_INLINE efviVoid deallocate(T* p, size_t n) const {
        EfviHugePages::instance->deallocate(p, n * sizeof(T));
    }

    template<class U>
    bool operator==(EfviHugePageAllocator<U>) const {
        return true;
    }

    template<class U>
    bool operator!=(EfviHugePageAllocator<U>) const {
        return false;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // HUGE_PAGES_H_INCLUDED


