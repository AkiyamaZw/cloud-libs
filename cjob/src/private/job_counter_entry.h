#pragma once
#include "job_define.h"
#include "icounter.h"
#include "resource_pool.h"

namespace cloud
{
struct JobWaitEntry;

class JobCounterEntry
    : public internal::IPoolableObject
    , public ICountable
{
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
    void add_dep_jobs(JobWaitEntry *entry);
    /* add counters that waiting this counter */
    void add_dep_counters(JobCounterEntry *counter);

    /* used while get from pool*/
    void init();
    /* called by the pool */
    void reset() override;
    /* a helper function to judge whether the count should be delete.*/
    bool ready_to_release() const;
    /* event callback */
    void on_counter_signal();
    void on_counter_destroyed();

    /* wait_job_list records [job, accumulate_counter] collection, which
    will be add to workers queue while cnt go to 0. */
    std::mutex dep_jobs_lock_;
    std::deque<JobWaitEntry *> wait_job_list_;

    /* wait_counter_list records counters that waitting current counters.*/
    std::mutex dep_counter_lock_;
    std::deque<JobCounterEntry *> wait_counter_list_;

    /* just the state of counter.*/
    std::atomic<State> state_{State::Released};
};
} // namespace cloud