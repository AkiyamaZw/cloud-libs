#pragma once
#include <cstdint>
#include <functional>
#include "job_define.h"
#include "job.h"
#include "counter.h"

namespace cloud::internal
{
template <typename T>
class ResourcePool;
}

namespace cloud
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

    void shutdown();
    bool is_active() const;
    bool has_active_job() const;
    uint32_t get_num_core() const;

    // thread attachment
    void adopt();
    void emancipate();

    // wait counter
    JobWaitEntry *create_job(const std::string &name,
                             JobFunc task,
                             JobCounterEntry *wait_counter,
                             JobCounterEntry *acc_counter);
    JobCounterEntry *create_entry_counter();

    void spin_wait(JobCounterEntry *counter);
    void spin_wait(const Counter &counter);
    void release_counter(JobCounterEntry *counter);
    void try_signal(JobCounterEntry *counter);
    Counter create_counter();

  protected:
    uint32_t dispatch_group_count(uint32_t job_count,
                                  uint32_t group_size) const;
    uint32_t calc_core_num(uint32_t core_num) const;

    void commit_job(JobWaitEntry *job_pkt);
    void release_job(JobWaitEntry *job_pkt);
    bool execute_job(Worker &worker);
    void try_dispatch(JobCounterEntry *counter);

  private:
    friend class JobQueueProxy;
    WorkerThreads *workers_{nullptr};
    std::unique_ptr<JobQueueProxy> queue_proxy_;
    using CounterPool = internal::ResourcePool<JobCounterEntry>;
    std::unique_ptr<CounterPool> counter_pool_{nullptr};
    using JobWaitListEntryPool = internal::ResourcePool<JobWaitEntry>;
    std::unique_ptr<JobWaitListEntryPool> entry_pool_{nullptr};
};
} // namespace cloud