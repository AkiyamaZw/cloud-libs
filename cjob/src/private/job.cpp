#include "job.h"
namespace cloud
{

void Job::Execute()
{
    JobArgs args;
    args.group_id = this->group_id;
    args.shared_memory =
        shared_memory_size > 0 ? alloca(shared_memory_size) : nullptr;

    for (uint32_t j = group_job_offset; j < group_job_end; ++j)
    {
        args.job_index = j;
        args.group_index = j - group_job_offset;
        args.is_first_job_in_group = (j == group_job_offset);
        args.is_last_job_in_group = (j == group_job_end - 1);
        task(args);
    }

    context->counter.fetch_sub(1);
}

void JobQueue::PushBack(const Job &job)
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    queue.push_back(job);
}

bool JobQueue::PopFront(Job &job)
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (queue.empty())
    {
        return false;
    }
    job = std::move(queue.front());
    queue.pop_front();
    return true;
}

void PriorityResource::Work(uint32_t starting_queue)
{
    Job job;
    for (uint32_t i = 0; i < num_thread; ++i)
    {
        auto job_queue = job_queues_per_thread[starting_queue % num_thread];
        while (job_queue->PopFront(job))
        {
            job.Execute();
        }
        starting_queue += 1;
    }
}

} // namespace cloud
