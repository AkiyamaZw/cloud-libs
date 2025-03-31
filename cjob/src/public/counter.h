#pragma once
#include "job_define.h"
#include "resource_pool.h"

namespace cloud
{
struct JobWaitListEntry;
class Counter;
class Job;
class JobSystem;

class JobCounterEntry
    : public internal::IPoolableObject
    , public ICountable
{
    friend class Counter;

  public:
    enum class State
    {
        Released,
        Setupped,
        FinishAddDepend
    };

  public:
    JobCounterEntry();
    ~JobCounterEntry();
    JobCounterEntry(const JobCounterEntry &) = delete;
    JobCounterEntry(JobCounterEntry &&) = delete;
    JobCounterEntry &operator=(const JobCounterEntry &) = delete;
    JobCounterEntry &operator=(JobCounterEntry &&) = delete;

    /* add job packet that waiting this counter */
    void add_dep_jobs(JobWaitListEntry *entry);
    /* add counters that waiting this counter */
    void add_dep_counters(JobCounterEntry *counter);

    /* used while get from pool*/
    void init();
    /* called by the pool */
    void reset() override;
    // a helper function to judge whether the count should be delete.
    bool ready_to_release() const;
    // event callback
    void on_counter_signal();
    void on_counter_destroyed();

    /* wait_job_list records [job, accumulate_counter] collection, which
    will be add to workers queue while cnt go to 0. */
    std::mutex dep_jobs_lock_;
    std::deque<JobWaitListEntry *> wait_job_list_;
    // std::vector<JobWaitListEntry *> wait_job_list_;
    // std::atomic<uint32_t> next_runable_jobs_{0};
    /* wait_counter_list records counters that waitting current counters.*/
    std::mutex dep_counter_lock_;
    std::deque<JobCounterEntry *> wait_counter_list_;
    // std::atomic<uint32_t> next_signable_counter_{0};
    /* just the state of counter.*/
    std::atomic<State> state_{State::Released};
};

class Counter
{
  public:
    Counter(JobCounterEntry *entry);
    ~Counter();
    Counter(const Counter &rhs);
    Counter(Counter &&rhs) noexcept;
    Counter &operator=(const Counter &rhs);
    Counter &operator=(Counter &&rhs);
    Counter &operator+=(const Counter &rhs);
    bool operator==(const Counter &rhs) const;

    constexpr bool is_valid() const { return entry_ != nullptr; }
    uint32_t get_cnt() const { return is_valid() ? entry_->get_cnt() : 0; }
    void add(uint32_t i = 1) { entry_->add_cnt(i); }
    void sub(uint32_t i = 1) { entry_->sub_cnt(i); }
    void set_cnt(uint32_t i) { entry_->set_cnt(i); }
    uint32_t get_id() const { return entry_->get_index(); }
    void finish_submit_job();
    JobCounterEntry *get_entry() const { return entry_; };
    void set_entry(JobCounterEntry *entry) { entry_ = entry; }

    static Counter create(JobSystem &js);

  private:
    JobCounterEntry *entry_{nullptr};
};

struct JobWaitListEntry : public internal::IPoolableObject
{
    JobCounterEntry *accumulate_counter{nullptr};
    Job *job{nullptr};

    void reset() override
    {
        accumulate_counter = nullptr;
        job = nullptr;
    }
};

} // namespace cloud