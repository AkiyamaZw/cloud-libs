#include "job_​scheduler.h"
#include "job_system.h"
#include "worker_threads.h"
#include "resource_pool.h"
#include "job.h"

namespace cloud::js
{
JobWaitEntry *Job​Scheduler::pop_job(JobQueue &queue)
{
    JobWaitEntry *entry = nullptr;

    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.pop();

    entry = !index ? nullptr : entry_pool().at(index - 1);
    if (entry == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            workers().try_wake_up(old_jobs);
        }
    }
    return entry;
}

void Job​Scheduler::push_job(Worker &worker, JobWaitEntry *job_pkt)
{
    assert(job_pkt != nullptr);
    // this means left 0 to invalid default.
    worker.job_queue.push(job_pkt->get_index() + 1);
    auto old_value = active_jobs_.fetch_add(1, std::memory_order_relaxed);
    if (old_value >= 0)
    {
        workers().try_wake_up(old_value + 1);
    }
}

JobWaitEntry *Job​Scheduler::steal_job(JobQueue &queue)
{
    JobWaitEntry *job_pkt{nullptr};
    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.steal();
    job_pkt = !index ? nullptr : entry_pool().at(index - 1);
    if (job_pkt == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            workers().try_wake_up(old_jobs);
        }
    }
    return job_pkt;
}

JobWaitEntry *Job​Scheduler::steal(Worker &worker)
{
    JobWaitEntry *job_pkt{nullptr};
    do
    {
        auto target_worker = workers().random_select(worker);
        if (target_worker)
        {
            job_pkt = steal_job(target_worker->job_queue);
        }
    } while (job_pkt == nullptr && get_active_jobs() > 0);
    return job_pkt;
}

JobWaitEntry *Job​Scheduler::fetch_job(Worker &worker)
{
    // 1. get job from this_thread
    auto *job_pkt = pop_job(worker.job_queue);
    // 2. if not get job from other threads
    if (job_pkt == nullptr)
    {
        job_pkt = steal(worker);
    }
    return job_pkt;
}
} // namespace cloud::js