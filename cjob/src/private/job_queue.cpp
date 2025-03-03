#include "job.h"

namespace cloud
{
void JobQueue::PushBack(Job &job)
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
    job = queue.front();
    queue.pop_front();
    return true;
}
} // namespace cloud