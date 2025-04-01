#pragma once
#include "job_define.h"
#include "icounter.h"
#include "resource_pool.h"

namespace cloud::js
{
struct JobWaitEntry;

class JobCounterEntry : public internal::IPoolableObject<JobCounterEntry>
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
    /* set state and do some state check */
    void set_state(State state);
    /* add wait cnt */
    void accumulate();
    /* sub wait cnt */
    void signal();
    /* get wait counts */
    uint32_t get_waits() const;

  public:
    /* ref counter */
    Countable ref_counter_;

    /* wait_job_list records [job, accumulate_counter] collection, which
    will be add to workers queue while cnt go to 0. */
    std::mutex dep_jobs_lock_;
    std::deque<JobWaitEntry *> wait_job_list_;

    /* wait_counter_list records counters that waitting current counters.*/
    std::mutex dep_counter_lock_;
    std::deque<JobCounterEntry *> wait_counter_list_;

  private:
    /* just the state of counter.*/
    std::atomic<State> state_{State::Released};

    /* mininal counter */
    Countable wait_counter_;
};
} // namespace cloud::js