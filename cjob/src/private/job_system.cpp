#include "job_system.h"
#include <cassert>
#include <algorithm>
#include "worker_threads.h"

namespace cloud
{

JobSystem::JobSystem(uint32_t max_thread_count, uint32_t max_adopt_thread)
{
    job_pool_ = std::make_unique<JobPool>();
    workers_ = std::make_unique<WorkerThreads>(
        calc_core_num(max_thread_count),
        max_adopt_thread,
        std::bind(JobSystem::execute_job, this, std::placeholders::_1));
}

JobSystem::~JobSystem()
{
    shutdown();
    workers_.reset();
    job_pool_.reset();
}

void JobSystem::shutdown()
{
    if (!is_active())
    {
        return;
    }

    workers_->toggle_alive(false);
}

bool JobSystem::is_active() const { return workers_->get_alive(); }

uint32_t JobSystem::get_num_core() const
{
    return workers_ ? workers_->num_thread_ : 0u;
}

bool JobSystem::has_active_job() const
{
    return active_jobs_.load(std::memory_order_relaxed) > 0;
}

Job *JobSystem::create(Job *parent, JobFunc sub_task)
{
    assert(parent != nullptr);
    if (parent == nullptr)
        return nullptr;

    // check parent is finished first!
    if (parent->is_completed())
        return nullptr;

    Job *sub_job = job_pool_->get();
    if (parent)
        parent->running_job_count_.fetch_add(1, std::memory_order_relaxed);
    sub_job->task = sub_task;
    return sub_job;
}

Job *JobSystem::create_parent_job(Job *parent)
{
    Job *sub_job = job_pool_->get();
    parent->running_job_count_.fetch_add(1, std::memory_order_relaxed);
    return sub_job;
}

void JobSystem::run(Job *job)
{
    auto worker = workers_->get_worker();
    push_job(worker->job_queue, job);
    job = nullptr;
}

void JobSystem::wait_and_release(Job *job)
{
    assert(job);
    assert(job->handle_.is_valid());
    auto worker = workers_->get_worker();
    assert(worker);
    do
    {

    } while (!job->is_completed() && is_active());
}

uint32_t JobSystem::dispatch_group_count(uint32_t job_count,
                                         uint32_t group_size) const
{
    return (job_count + group_size - 1) / group_size;
}

uint32_t JobSystem::calc_core_num(uint32_t max_core_num) const
{

    max_core_num = std::max(1u, max_core_num);
    uint32_t num_core = std::thread::hardware_concurrency();
    return std::clamp(num_core - 1, 1u, max_core_num);
}

void JobSystem::release_job(Job *job) { job_pool_->release(job); }

bool JobSystem::execute_job(Worker &worker)
{
    Job *job = pop_job(worker.job_queue);
    if (job == nullptr)
    {
        job = steal(worker);
    }
    if (job)
    {
        // 1. check job is running?
        if (!job->is_completed() && job->task)
        {
            job->Execute();
        }
    }
    return job != nullptr;
}

void JobSystem::push_job(JobQueue &queue, Job *job)
{
    assert(job != nullptr);
    assert(job->handle_.is_valid());
    // this means left 0 to invalid default.
    queue.push(job->handle_.index + 1);
    auto old_value = active_jobs_.fetch_add(1, std::memory_order_relaxed);
    if (old_value >= 0)
    {
        workers_->try_wake_up(old_value + 1);
    }
}

Job *JobSystem::pop_job(JobQueue &queue)
{
    Job *job = nullptr;

    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.pop();

    job = !index ? nullptr : job_pool_->at(index - 1);
    if (job == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            workers_->try_wake_up(old_jobs);
        }
    }
    return job;
}

Job *JobSystem::steal_job(JobQueue &queue)
{
    Job *job{nullptr};
    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.steal();
    job = !index ? nullptr : job_pool_->at(index - 1);
    if (job == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            workers_->try_wake_up(old_jobs);
        }
    }
    return job;
}

Job *JobSystem::steal(Worker &worker)
{
    Job *job{nullptr};
    do
    {
        auto target_worker = workers_->random_select(worker);
        if (target_worker)
        {
            job = steal_job(target_worker->job_queue);
        }
    } while (job == nullptr && has_active_job());
    return job;
}

} // namespace cloud