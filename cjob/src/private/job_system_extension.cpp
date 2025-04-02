#include "job_system_extension.h"
#include "job_system.h"
#include "worker_threads.h"
#include "resource_pool.h"
#include "job.h"

namespace cloud::js
{
JobSystemExtension::JobSystemExtension(JobSystem *js)
    : js_(js)
{
    assert(js_ != nullptr);
}

JobSystemExtension::~JobSystemExtension() { js_ = nullptr; }

JobWaitEntry *JobQueueProxy::pop_job(JobQueue &queue)
{
    JobWaitEntry *entry = nullptr;

    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.pop();

    entry = !index ? nullptr : js_->entry_pool_->at(index - 1);
    if (entry == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            js_->workers_->try_wake_up(old_jobs);
        }
    }
    return entry;
}

void JobQueueProxy::push_job(JobQueue &queue, JobWaitEntry *job_pack)
{
    assert(job_pack != nullptr);
    // this means left 0 to invalid default.
    queue.push(job_pack->get_index() + 1);
    auto old_value = active_jobs_.fetch_add(1, std::memory_order_relaxed);
    if (old_value >= 0)
    {
        js_->workers_->try_wake_up(old_value + 1);
    }
}

JobWaitEntry *JobQueueProxy::steal_job(JobQueue &queue)
{
    JobWaitEntry *job_pkt{nullptr};
    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.steal();
    job_pkt = !index ? nullptr : js_->entry_pool_->at(index - 1);
    if (job_pkt == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            js_->workers_->try_wake_up(old_jobs);
        }
    }
    return job_pkt;
}

JobWaitEntry *JobQueueProxy::steal(Worker &worker)
{
    JobWaitEntry *job_pkt{nullptr};
    do
    {
        assert(js_->workers_ != nullptr);
        auto target_worker = js_->workers_->random_select(worker);
        if (target_worker)
        {
            job_pkt = steal_job(target_worker->job_queue);
        }
    } while (job_pkt == nullptr && js_->has_active_job());
    return job_pkt;
}
} // namespace cloud::js