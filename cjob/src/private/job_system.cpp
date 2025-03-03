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
    if (IsBusy(context))
    {

        workers->wake_condition.notify_all();
        workers->Work(workers->next_queue.fetch_add(1) % workers->num_thread_);
        while (IsBusy(context))
        {
            std::this_thread::yield();
        }
    }
}

void JobSystem::Execute(JobContext &context, const JobFunc &task)
{

    context.counter.fetch_add(1);
    Job job;
    job.context = &context;
    job.task = task;
    job.group_id = 0;
    job.group_job_offset = 0;
    job.group_job_end = 1;
    job.shared_memory_size = 0;

    if (workers->num_thread_ < 1)
    {
        job.Execute();
        return;
    }
    workers
        ->job_queues_per_thread[workers->next_queue.fetch_add(1) %
                                workers->num_thread_]
        ->PushBack(job);
    workers->wake_condition.notify_one();
}

void JobSystem::Dispatch(JobContext &context,
                         uint32_t job_count,
                         uint32_t group_size,
                         const JobFunc &task,
                         size_t shared_memory_size)
{
    if (job_count == 0 || group_size == 0)
    {
        return;
    }

    const uint32_t group_count = dispatch_group_count(job_count, group_size);

    std::atomic_fetch_add(&context.counter, group_count);

    Job job;
    job.context = &context;
    job.task = task;
    job.shared_memory_size = (uint32_t)shared_memory_size;
    for (uint32_t group_id = 0; group_id < group_count; ++group_id)
    {
        job.group_id = group_id;
        job.group_job_offset = group_id;
        job.group_job_end =
            std::min(job.group_job_offset + group_size, job_count);

        if (workers->num_thread_ < 1)
        {
            job.Execute();
        }
        else
        {
            workers
                ->job_queues_per_thread[workers->next_queue.fetch_add(1) %
                                        workers->num_thread_]
                ->PushBack(job);
        }
    }
    if (workers->num_thread_ > 1)
    {
        workers->wake_condition.notify_all();
    }
}

bool JobSystem::IsActive() const { return workers->get_alive(); }

bool JobSystem::IsBusy(const JobContext &context) const
{
    return context.counter.load() > 0;
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
} // namespace cloud