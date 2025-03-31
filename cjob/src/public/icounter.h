#pragma once
#include <cassert>
#include <atomic>
#include <cstdint>

class ICountable
{
  public:
    ICountable() = default;
    virtual ~ICountable() { assert(cnt.load() == 0); }
    std::atomic_uint16_t cnt{0};
    uint16_t add_cnt(uint16_t i = 1) { return cnt.fetch_add(i); }
    uint16_t sub_cnt(uint16_t i = 1) { return cnt.fetch_sub(i); }
    void set_cnt(uint16_t i = 0) { cnt.store(i); }
    uint16_t get_cnt() const { return cnt.load(); }
};