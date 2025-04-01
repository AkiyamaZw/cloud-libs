#pragma once
#include <cassert>
#include <atomic>
#include <cstdint>

namespace cloud::js
{
class Countable
{
  public:
    using SignalCallback = std::function<void(Countable &)>;
    Countable() = default;
    virtual ~Countable() { assert(cnt_.load() == 0); }

    uint16_t add_cnt(uint16_t i = 1) { return cnt_.fetch_add(i); }
    uint16_t sub_cnt(uint16_t i = 1)
    {
        assert(cnt_ >= i);
        uint16_t latest = cnt_.fetch_sub(i);
        auto iter = signal_callback_.find(latest - 1);
        if (iter != signal_callback_.end())
        {
            iter->second(*this);
        }
        return latest;
    }
    void set_cnt(uint16_t i = 0) { cnt_.store(i); }
    uint16_t get_cnt() const { return cnt_.load(); }

    void set_signal_callback(uint16_t id, SignalCallback cb)
    {
        signal_callback_[id] = cb;
    };

  private:
    std::atomic_uint16_t cnt_{0};
    std::unordered_map<uint16_t, SignalCallback> signal_callback_;
};

} // namespace cloud::js
