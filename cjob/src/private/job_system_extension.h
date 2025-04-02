#pragma once
#include "job_define.h"

namespace cloud::js
{
class JobSystem;
class JobSystemExtension
{
  public:
    using Super = JobSystemExtension;
    JobSystemExtension(JobSystem *js);
    virtual ~JobSystemExtension();

  protected:
    JobSystem *js_;
};

class JobWaitEntry;
class Worker;

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
} // namespace cloud::js