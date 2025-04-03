#pragma once
#include "job_define.h"
#include "countable.h"
#include "resource_pool.h"

namespace cloud::js
{
class JobWaitEntry;

class SubToTriggerCountable : public Countable
{
  public:
    using Callback = std::function<void(JobCounterEntry *)>;

    SubToTriggerCountable(uint32_t level = 0);
    ~SubToTriggerCountable();

    void register_callback(const std::string &name, Callback callback);

    void unregister_callback(const std::string &name);
    uint16_t sub_cnt(uint16_t i = 1, JobCounterEntry *entry = nullptr);
    void reset() override
    {
        Countable::reset();
        std::lock_guard lock(map_mutex_);
        callbacks_.clear();
    }

  private:
    uint32_t level_{0};
    std::mutex map_mutex_;
    std::unordered_map<std::string, Callback> callbacks_;
};

class JobCounterEntry
    : public internal::CountablePoolableObject<JobCounterEntry>
{
  public:
    enum class State
    {
        Released,
        Setupped,
        FinishAddDepend
    };

  protected:
    using Callback = SubToTriggerCountable::Callback;

  public:
    JobCounterEntry();
    ~JobCounterEntry();

    JobCounterEntry(const JobCounterEntry &) = delete;
    JobCounterEntry(JobCounterEntry &&) = delete;
    JobCounterEntry &operator=(const JobCounterEntry &) = delete;
    JobCounterEntry &operator=(JobCounterEntry &&) = delete;

    /* add job packet that waiting this counter */
    void add_dep_jobs(JobWaitEntry *entry);
    /* add counters that waiting this counter */
    void add_dep_counters(JobCounterEntry *counter);

    /* used while get from pool*/
    void init();
    /* used with sub to zero counter */
    void init(Callback callback);
    /* called by the pool */
    void reset() override;
    /* a helper function to judge whether the count should be delete.*/
    bool ready_to_release() const;
    /* event callback */
    void on_counter_signal();
    void on_counter_destroyed();
    /* add wait cnt */
    void accumulate();
    /* sub wait cnt */
    void signal();
    /* get wait counts */
    uint32_t get_waits() const;

  public:
    /* wait_job_list records [job, accumulate_counter] collection, which
    will be add to workers queue while cnt go to 0. */
    std::mutex dep_jobs_lock_;
    std::deque<JobWaitEntry *> wait_job_list_;

    /* wait_counter_list records counters that waitting current counters.*/
    std::mutex dep_counter_lock_;
    std::deque<JobCounterEntry *> wait_counter_list_;
    std::atomic<bool> finish_submit_job_{false};

  private:
    /* mininal counter */
    SubToTriggerCountable wait_counter_;
};
} // namespace cloud::js