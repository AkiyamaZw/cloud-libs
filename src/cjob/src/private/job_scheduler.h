#pragma once
#include "job_system_extension.h"

namespace cloud::js
{
class JobWaitEntry;
class Worker;

/* IScheduler works like a job queue, that makes sure different worker working
 * in a ​efficient way */
class IScheduler
{
  public:
    /* send job to scheduler*/
    virtual void push_job(Worker &worker, JobWaitEntry *job_pkt) = 0;
    /* fetch one job according to job workers*/
    virtual JobWaitEntry *fetch_job(Worker &worker) = 0;
    /* how many jobs left in scheduler queue*/
    virtual uint32_t get_active_jobs() const = 0;
};

/* current: steal use random strategy, and only one queue per worker. priority
 * should implement in this place */
class JobScheduler
    : public JobSystemProxy
    , public IScheduler
{
  public:
    using Super::JobSystemProxy;
    /* send job to scheduler*/
    void push_job(Worker &worker, JobWaitEntry *job_pkt) override;
    /* fetch one job according to job workers*/
    JobWaitEntry *fetch_job(Worker &worker) override;
    /* how many jobs left in scheduler queue*/
    uint32_t get_active_jobs() const override { return active_jobs_.load(); };

  private:
    JobWaitEntry *pop_job(JobQueue &queue);
    JobWaitEntry *steal_job(JobQueue &queue);
    JobWaitEntry *steal(Worker &worker);

  private:
    std::atomic<uint32_t> active_jobs_{0};
};
} // namespace cloud::js