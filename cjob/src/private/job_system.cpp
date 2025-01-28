#include "job_system.h"
#include <cassert>
#include <algorithm>

namespace cloud
{

JobSystem::~JobSystem() { Shutdown(); }

void JobSystem::Active(uint32_t max_thread_count)
{
    if (num_core > 0)
    {
        return;
    }
    max_thread_count = std::max(1u, max_thread_count);
    num_core = std::thread::hardware_concurrency();
    for (int i = 0; i < int(JobPriority::Count); ++i)
    {
        const JobPriority priority = static_cast<JobPriority>(i);
        PriorityResource &res = resources[i];

        if (priority == JobPriority::High)
        {
            res.num_thread = num_core - 1; // -1 for main thread
        }
        else if (priority == JobPriority::Low)
        {
            res.num_thread = num_core - 2; // -2 for main thread, -1 for stream
        }
        else if (priority == JobPriority::Streaming)
        {
            res.num_thread = 1;
        }
        else
        {
            assert(false);
        }
        res.num_thread = std::clamp(res.num_thread, 1u, max_thread_count);
        for (int thread_id = 0; thread_id < res.num_thread; ++thread_id)
        {
            res.job_queues_per_thread.push_back(std::make_shared<JobQueue>());
        }
        res.threads.reserve(res.num_thread);

        for (uint32_t thread_id = 0; thread_id < res.num_thread; ++thread_id)
        {
            std::thread &worker =
                res.threads.emplace_back([this, thread_id, &res]() {
                    while (alive.load())
                    {
                        res.Work(thread_id);
                        std::unique_lock<std::mutex> lock(res.wake_mutex);
                        res.wake_condition.wait(lock);
                    }
                });
            auto handle = worker.native_handle();
            int core = thread_id + 1;
            if (priority == JobPriority::Streaming)
            {
                core = num_core - 1 - thread_id;
            }
        }
    }
}

void JobSystem::Shutdown()
{
    if (!IsActive())
    {
        return;
    }

    alive.store(false);
    bool wake_loop = true;
    std::thread waker([&]() {
        while (wake_loop)
        {
            for (auto &x : resources)
            {
                x.wake_condition.notify_all();
            }
        }
    });
    for (auto &x : resources)
    {
        for (auto &thread : x.threads)
        {
            thread.join();
        }
    }
    wake_loop = false;
    waker.join();
    for (auto &x : resources)
    {
        x.job_queues_per_thread.clear();
        x.threads.clear();
        x.num_thread = 0;
    }
}

void JobSystem::Wait(const JobContext &context)
{
    if (IsBusy(context))
    {
        PriorityResource &res = resources[int(context.priority)];
        res.wake_condition.notify_all();
        res.Work(res.next_queue.fetch_add(1) % res.num_thread);
        while (IsBusy(context))
        {
            std::this_thread::yield();
        }
    }
}

void JobSystem::Execute(JobContext &context,
                        const std::function<void(JobArgs)> &task)
{
    PriorityResource &res = resources[int(context.priority)];
    context.counter.fetch_add(1);
    Job job;
    job.context = &context;
    job.task = task;
    job.group_id = 0;
    job.group_job_offset = 0;
    job.group_job_end = 1;
    job.shared_memory_size = 0;

    if (res.num_thread < 1)
    {
        job.Execute();
        return;
    }
    res.job_queues_per_thread[res.next_queue.fetch_add(1) % res.num_thread]
        ->PushBack(job);
    res.wake_condition.notify_one();
}

void JobSystem::Dispatch(JobContext &context,
                         uint32_t job_count,
                         uint32_t group_size,
                         const std::function<void(JobArgs)> &task,
                         size_t shared_memory_size)
{
    if (job_count == 0 || group_size == 0)
    {
        return;
    }
    PriorityResource &res = resources[int(context.priority)];
    const uint32_t group_count = DispatchGroupCount(job_count, group_size);

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

        if (res.num_thread < 1)
        {
            job.Execute();
        }
        else
        {
            res.job_queues_per_thread[res.next_queue.fetch_add(1) %
                                      res.num_thread]
                ->PushBack(job);
        }
    }
    if (res.num_thread > 1)
    {
        res.wake_condition.notify_all();
    }
}

bool JobSystem::IsActive() const { return alive.load(); }

bool JobSystem::IsBusy(const JobContext &context) const
{
    return context.counter.load() > 0;
}

uint32_t JobSystem::DispatchGroupCount(uint32_t job_count,
                                       uint32_t group_size) const
{
    return (job_count + group_size - 1) / group_size;
}
} // namespace cloud