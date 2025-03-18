#pragma once
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <string>
#include <set>

namespace cloud
{
struct CJob;
class JobCounterEntry
{
    friend class Counter;

  private:
    JobCounterEntry() = default;
    ~JobCounterEntry() = default;
    JobCounterEntry(const JobCounterEntry &) = delete;
    JobCounterEntry(JobCounterEntry &&) = delete;
    JobCounterEntry &operator=(const JobCounterEntry &) = delete;
    JobCounterEntry &operator=(JobCounterEntry &&) = delete;
    static JobCounterEntry *create()
    {
        JobCounterEntry *entry = new JobCounterEntry();
        entry->ref_.fetch_add(1, std::memory_order_relaxed);
        return entry;
    };

    static void release(JobCounterEntry *entry)
    {
        if (entry != nullptr)
        {
            auto x = entry->ref_.fetch_sub(1);
            if (x == 1)
            {
                delete entry;
                entry = nullptr;
            }
        }
    }

    void add_dep_jobs(CJob *job);
    void add_dep_jobs(std::set<CJob *> jobs);
    std::atomic_uint16_t cnt_{0};
    std::atomic_uint16_t ref_{0};
    std::set<CJob *> dep_jobs_{};
    std::mutex dep_jobs_lock_;
};

class Counter
{
  public:
    Counter();
    ~Counter();
    Counter(const Counter &rhs);
    Counter(Counter &&rhs) noexcept;
    Counter &operator=(const Counter &rhs);
    Counter &operator==(Counter &&rhs);
    Counter &operator+=(const Counter &rhs);

    constexpr bool is_valid() const { return entry_ != nullptr; }
    uint32_t get_cnt() const { return is_valid() ? entry_->cnt_.load() : 0; }
    uint32_t get_ref() const { return is_valid() ? entry_->ref_.load() : 0; }
    void add(uint32_t i = 1) { entry_->cnt_.fetch_add(1); }
    void sub(uint32_t i = 0) { entry_->cnt_.fetch_sub(1); }

  private:
    JobCounterEntry *entry_{nullptr};
};

class JobBuilder
{
  public:
    using JobFunc = std::function<void()>;

    ~JobBuilder();
    // new job to dispatch
    void dispatch(const std::string &name, JobFunc func);
    // export wait 'cnt' of wait count, and turn wait counter's state to invalid
    Counter extract_wait_counter();
    // create a fence among builder's scope
    void dispatch_fence_explicitly();
    // connect jobbuilder by using counter. always used after
    void dispatch_wait(const Counter &counter);

  private:
    Counter wait_counter_;
    Counter accumulate_counter_;
};
} // namespace cloud