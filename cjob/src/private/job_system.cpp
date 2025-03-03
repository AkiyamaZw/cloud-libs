#include "job_system.h"
#include <cassert>
#include <algorithm>
#include "worker_threads.h"

namespace cloud
{

JobSystem::JobSystem(uint32_t max_thread_count)
{
    workers = std::make_unique<WorkerThreads>(calc_core_num(max_thread_count));
}

JobSystem::~JobSystem() { Shutdown(); }

void JobSystem::Shutdown()
{
    if (!IsActive())
    {
        return;
    }

    workers->toggle_alive(false);
}

void JobSystem::Wait(const JobContext &context)
{
    // if (IsBusy(context))
    // {

    //     workers->wake_condition.notify_all();
    //     workers->Work(workers->next_queue.fetch_add(1) %
    //     workers->num_thread_); while (IsBusy(context))
    //     {
    //         std::this_thread::yield();
    //     }
    // }
}

bool JobSystem::IsActive() const { return workers->get_alive(); }

bool JobSystem::IsBusy(const JobContext &context) const
{
    return context.counter.load() > 0;
}

uint32_t JobSystem::get_num_core() const
{
    return workers ? workers->num_thread_ : 0u;
}

Job *JobSystem::create(Job *parent, const JobFunc &sub_task)
{
    assert(parent != nullptr);
    if (parent == nullptr)
        return nullptr;

    // check parent is finished first!
    if (parent->is_completed())
        return nullptr;

    Job *sub_job = Job::create();
    parent->running_job_count_.fetch_add(1, std::memory_order_relaxed);
    sub_job->task = sub_task;
    return sub_job;
}

Job *JobSystem::create_parent_job(Job *parent)
{
    Job *sub_job = new Job();
    parent->running_job_count_.fetch_add(1, std::memory_order_relaxed);
    return sub_job;
}

void JobSystem::run(Job *job)
{
    auto &worker = workers->get_worker();
    workers->put(worker, job);
    job = nullptr;
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

void JobSystem::wait(std::unique_lock<std::mutex> &lock, Job *job)
{
    workers->wake_condition.wait(lock);
}

} // namespace cloud