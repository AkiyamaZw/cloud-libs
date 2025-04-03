#pragma once
#include <cassert>
#include <atomic>
#include <cstdint>
#include <string>
#include <mutex>
#include <unordered_map>
#include <functional>

namespace cloud::js
{
class Countable
{
  public:
    Countable() = default;
    virtual ~Countable() { assert(cnt_.load() == 0); }

    uint16_t add_cnt(uint16_t i = 1) { return cnt_.fetch_add(i); }
    virtual uint16_t sub_cnt(uint16_t i = 1)
    {
        assert(cnt_ >= i);
        return cnt_.fetch_sub(i);
    }
    void set_cnt(uint16_t i = 0) { cnt_.store(i); }
    uint16_t get_cnt() const { return cnt_.load(); }
    virtual void reset() { set_cnt(0); };

  private:
    std::atomic_uint16_t cnt_{0};
};

} // namespace cloud::js
