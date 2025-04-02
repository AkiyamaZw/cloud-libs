#pragma once
#include <cstdint>
#include <functional>
#include "job_define.h"
#include "job.h"
#include "counter.h"

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
class JobSystem;
struct JobWaitEntry;

class JobSystemExtension
{
  public:
    using Super = JobSystemExtension;
    JobSystemExtension(JobSystem *js);
    virtual ~JobSystemExtension();

  protected:
    JobSystem *js_;
};

class JobQueueProxy : public JobSystemExtension
{
  public:
    using Super::JobSystemExtension;

    JobWaitEntry *pop_job(JobQueue &queue);
    void push_job(JobQueue &queue, JobWaitEntry *job_pack);
    JobWaitEntry *steal_job(JobQueue &queue);
    JobWaitEntry *steal(Worker &worker);
    int32_t get_active_jobs() const { return active_jobs_.load(); };

  private:
    std::atomic<int32_t> active_jobs_{0};
};

class JobSystemExporter : public JobSystemExtension
{
  public:
    using Super::JobSystemExtension;
    void export_grapviz(const std::string &path);

  private:
    std::string counter_grapviz(const JobWaitEntry *counter);
    std::string job_graphviz(const Job &job) const;
};

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

  private:
    /* a threads manager */
    WorkerThreads *workers_{nullptr};
    /* a job queue proxy */
    friend class JobQueueProxy;
    std::unique_ptr<JobQueueProxy> queue_proxy_;
    /* JobCounterEntry's pool */
    using CounterPool = internal::ResourcePool<JobCounterEntry>;
    std::unique_ptr<CounterPool> counter_pool_{nullptr};
    /* JobWaitList's pool */
    using JobWaitListEntryPool = internal::ResourcePool<JobWaitEntry>;
    std::unique_ptr<JobWaitListEntryPool> entry_pool_{nullptr};
};
} // namespace cloud::js