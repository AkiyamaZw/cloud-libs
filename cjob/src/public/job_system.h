#pragma once
#include <cstdint>
#include <functional>
#include "job_define.h"
#include "job.h"

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
struct JobWaitListEntry;

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

    JobWaitListEntry *pop_job(JobQueue &queue);
    void push_job(JobQueue &queue, JobWaitListEntry *job_pack);
    JobWaitListEntry *steal_job(JobQueue &queue);
    JobWaitListEntry *steal(Worker &worker);
};

// todo
// class JobSystemExporter : public JobSystemExtension
// {
//   public:
//     using Super::JobSystemExtension;
//     void export_grapviz(const std::string &path);

//   private:
//     std::string counter_grapviz(const JobWaitListEntry *counter);
//     std::string job_graphviz(const Job &job) const;
// };

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
    JobWaitListEntry *create_job_packet(const std::string &name,
                                        JobFunc task,
                                        JobCounterEntry *wait_counter,
                                        JobCounterEntry *acc_counter);
    JobCounterEntry *create_entry_counter();

    void try_dispatch(JobCounterEntry *counter);
    void spin_wait(JobCounterEntry *counter);
    void release_counter(JobCounterEntry *counter);
    void try_signal(JobCounterEntry *counter);

  protected:
    uint32_t dispatch_group_count(uint32_t job_count,
                                  uint32_t group_size) const;
    uint32_t calc_core_num(uint32_t core_num) const;

    void run(JobWaitListEntry *job_pkt);
    void release_job(JobWaitListEntry *job_pkt);

    bool execute_job(Worker &worker);

    void wait_and_release(JobWaitListEntry *job_pkt);
    void run_and_wait(JobWaitListEntry *job_pkt);

  public:
    using JobPool = internal::ResourcePool<Job>;
    std::unique_ptr<JobPool> job_pool_{nullptr};
    using CounterPool = internal::ResourcePool<JobCounterEntry>;
    std::unique_ptr<CounterPool> counter_pool_{nullptr};
    using JobWaitListEntryPool = internal::ResourcePool<JobWaitListEntry>;
    std::unique_ptr<JobWaitListEntryPool> entry_pool_{nullptr};

  private:
    friend class JobQueueProxy;
    WorkerThreads *workers_{nullptr};
    std::atomic<int32_t> active_jobs_{0};
    std::unique_ptr<JobQueueProxy> queue_proxy_;
};
} // namespace cloud