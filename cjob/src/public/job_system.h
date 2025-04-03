#pragma once
#include "job_define.h"

namespace cloud::js::internal
{
template <typename T>
class ResourcePool;
}

namespace cloud::js
{
class WorkerThreads;
struct Worker;
class JobCounterEntry;
class JobWaitEntry;
class Counter;
class I​Scheduler;

class JobSystem final
{
  public:
    JobSystem(uint32_t max_thread_count = 1, uint32_t max_adopt_thread = 5);
    ~JobSystem();

    bool is_active() const;
    bool has_active_job() const;
    uint32_t get_num_core() const;

    /* attch current thread, and add a job queue to this thread */
    void adopt();
    /* detach current thread */
    void emancipate();

    /* create a jobwait entry */
    JobWaitEntry *create_job(const std::string &name,
                             JobFunc task,
                             JobPriority property,
                             JobCounterEntry *wait_counter,
                             JobCounterEntry *acc_counter);
    /* create a counter entry */
    JobCounterEntry *create_entry_counter();
    /* signal a counter. */
    void try_signal(JobCounterEntry *counter);

    /* watting in user space util counter counts to zero*/
    void spin_wait(const Counter &counter);

  protected:
    /* make core_num in the range from 1 to hardware_concurrency */
    static uint32_t calc_core_num(uint32_t core_num);

    /* waits count to zero already and commit job to queue, if not call
     * try_signal*/
    void try_dispatch_job(JobCounterEntry *counter);

    /* fetch job and run job. this function run in worker threads */
    bool execute_job(Worker &worker);

    /* try dispatch accumulate counters and release jobs*/
    void after_job_execute(JobWaitEntry *job_pkt);

  protected:
    /* stop all works and wait all workers stop */
    void shutdown();

  protected:
    /* a threads manager */
    WorkerThreads *workers_{nullptr};
    /* a job queue proxy */
    friend class JobSystemProxy;
    std::unique_ptr<I​Scheduler> queue_proxy_;
    /* JobCounterEntry's pool */
    std::unique_ptr<CounterPool> counter_pool_{nullptr};
    /* JobWaitList's pool */
    std::unique_ptr<JobWaitListEntryPool> entry_pool_{nullptr};
};
} // namespace cloud::js