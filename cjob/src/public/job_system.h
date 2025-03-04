#pragma once
#include <cstdint>
#include <functional>
#include "job_define.h"
#include "job.h"

namespace cloud
{
class WorkerThreads;
struct Worker;

class JobSystem final
{
  public:
    JobSystem(uint32_t max_thread_count = 0, uint32_t max_adopt_thread = 5);
    ~JobSystem();

    void shutdown();
    bool is_active() const;
    bool has_active_job() const;
    uint32_t get_num_core() const;

    // thread attachment
    void adopt();
    void emancipate();

    // job operater
    JobHandle create(JobHandle parent, JobFunc sub_task);
    JobHandle create(JobFunc sub_task);
    void run(JobHandle job_handle);
    void wait_and_release(JobHandle job_handle);
    void run_and_wait(JobHandle job_handle);

  protected:
    uint32_t dispatch_group_count(uint32_t job_count,
                                  uint32_t group_size) const;
    uint32_t calc_core_num(uint32_t core_num) const;
    Job *create(Job *parent, JobFunc sub_task);
    void finish_job(Job *job);
    void run(Job *job);
    void release_job(Job *job);

    bool execute_job(Worker &worker);
    Job *pop_job(JobQueue &queue);
    void push_job(JobQueue &queue, Job *job);
    Job *steal_job(JobQueue &queue);
    Job *steal(Worker &worker);

    void wait_and_release(Job *job);
    void run_and_wait(Job *job);

  private:
    WorkerThreads *workers_{nullptr};
    std::unique_ptr<JobPool> job_pool_{nullptr};
    std::atomic<int32_t> active_jobs_{0};
};
} // namespace cloud