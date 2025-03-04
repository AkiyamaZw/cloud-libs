#include "job_system.h"
#include <cassert>
#include <algorithm>
#include "worker_threads.h"

namespace cloud
{

JobSystem::JobSystem(uint32_t max_thread_count, uint32_t max_adopt_thread)
{
    job_pool_ = std::make_unique<JobPool>();
    workers_ = new WorkerThreads();
    workers_->init(
        calc_core_num(max_thread_count),
        max_adopt_thread,
        std::bind(&JobSystem::execute_job, this, std::placeholders::_1));
}

JobSystem::~JobSystem()
{
    shutdown();
    delete workers_;
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
    return workers_ ? workers_->get_thread_num() : 0u;
}

bool JobSystem::has_active_job() const
{
    return active_jobs_.load(std::memory_order_relaxed) > 0;
}

void JobSystem::adopt() { workers_->attach(); }

void JobSystem::emancipate() { workers_->detach(); }

Job *JobSystem::create(Job *parent, JobFunc sub_task)
{
    // check parent is finished first!
    if (parent != nullptr && parent->is_completed())
        return nullptr;

    Job *sub_job = job_pool_->get();
    if (parent)
    {
        parent->running_job_count_.fetch_add(1, std::memory_order_relaxed);
        sub_job->parent_ = parent->handle_;
    }
    sub_job->task = sub_task;
    return sub_job;
}

void JobSystem::run(Job *job)
{
    auto worker = workers_->get_worker();
    push_job(worker->job_queue, job);
}

void JobSystem::wait_and_release(Job *job)
{
    assert(job);
    assert(job->handle_.is_valid());
    auto worker = workers_->get_worker();
    assert(worker != nullptr);
    do
    {
        if (!execute_job(*worker))
        {
            if (job->is_completed())
            {
                break;
            }
            // this section means the job is running in other processor
            if (!job->is_completed() && !has_active_job() && is_active())
            {
                workers_->wait(job);
            }
        }
    } while (!job->is_completed() && is_active());
    job_pool_->release(job);
}

void JobSystem::run_and_wait(Job *job)
{
    assert(job != nullptr);
    run(job);
    wait_and_release(job);
    job = nullptr;
}

JobHandle JobSystem::create(JobHandle parent, JobFunc sub_task)
{
    Job *parent_job = parent.is_valid() ? job_pool_->at(parent) : nullptr;
    Job *new_job = create(parent_job, sub_task);
    return new_job ? new_job->handle_ : Job::INVALID_HANDLE;
}

JobHandle JobSystem::create(JobFunc sub_task)
{
    return create(Job::INVALID_HANDLE, sub_task);
}

void JobSystem::run(JobHandle job_handle)
{
    if (!job_handle.is_valid())
        return;
    run(job_pool_->at(job_handle));
}

void JobSystem::wait_and_release(JobHandle job_handle)
{
    if (!job_handle.is_valid())
        return;
    wait_and_release(job_pool_->at(job_handle));
}

void JobSystem::run_and_wait(JobHandle job_handle)
{
    if (!job_handle.is_valid())
    {
        return;
    }
    run_and_wait(job_pool_->at(job_handle));
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
            JobArgs args{};
            job->task(args);
        }
        finish_job(job);
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
        assert(workers_ != nullptr);
        auto target_worker = workers_->random_select(worker);
        if (target_worker)
        {
            job = steal_job(target_worker->job_queue);
        }
    } while (job == nullptr && has_active_job());
    return job;
}

void JobSystem::finish_job(Job *job)
{
    bool notify = false;
    do
    {
        auto runing_job_count =
            job->running_job_count_.fetch_sub(1, std::memory_order_acq_rel);
        assert(runing_job_count > 0);
        if (runing_job_count == 1)
        {
            notify = true;
            Job *parent =
                job->parent_.is_valid() ? job_pool_->at(job->parent_) : nullptr;
            job = parent;
        }
        else
        {
            break;
        }
    } while (job);
    if (notify)
    {
        workers_->try_wake_up_all();
    }
}

} // namespace cloud