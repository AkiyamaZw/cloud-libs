#pragma once
#include <cstddef>
#include <atomic>
#include <cassert>

namespace cloud
{

template <typename T, size_t CAPABILITY>
class StealableQueue
{
    static_assert(!(CAPABILITY & (CAPABILITY - 1)),
                  "CAPABILITY must be power of 2");
    static constexpr size_t MASK = CAPABILITY - 1;
    using index_type = int64_t;

  public:
    StealableQueue() = default;
    void push(T t);
    T pop();
    T steal();

  protected:
    T get_at(index_type index) noexcept { return queue_[index & MASK]; }

    void set_at(index_type index, T value) noexcept
    {
        queue_[index & MASK] = value;
    }

  private:
    T queue_[CAPABILITY];
    std::atomic<index_type> top_{0};
    std::atomic<index_type> bottom_{0};
};

template <typename T, size_t CAPABILITY>
inline void StealableQueue<T, CAPABILITY>::push(T t)
{
    index_type bottom = bottom_.load(std::memory_order_relaxed);
    set_at(bottom, t);
    bottom_.fetch_add(1, std::memory_order_release);
}

template <typename T, size_t CAPABILITY>
inline T StealableQueue<T, CAPABILITY>::pop()
{
    index_type bottom = bottom_.fetch_sub(1, std::memory_order_seq_cst) - 1;
    assert(bottom >= -1);
    index_type top = top_.load(std::memory_order_seq_cst);
    if (top < bottom)
    {
        return get_at(bottom);
    }
    T t{};
    if (top == bottom)
    {
        t = get_at(bottom);
        if (top_.compare_exchange_strong(top,
                                         top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed))
        {
            top++;
        }
        else
        {
            t = T();
        }
    }
    else
    {
        assert(top - bottom == 1);
    }
    bottom_.store(top, std::memory_order_relaxed);
    return t;
}

template <typename T, size_t CAPABILITY>
inline T StealableQueue<T, CAPABILITY>::steal()
{
    while (true)
    {
        index_type top = top_.load(std::memory_order_seq_cst);
        index_type bottom = bottom_.load(std::memory_order_seq_cst);
        // empty queue
        if (top >= bottom)
        {
            return T{};
        }

        T t = get_at(top);
        if (top_.compare_exchange_strong(top,
                                         top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed))
        {
            return t;
        }
    }
}
} // namespace cloud