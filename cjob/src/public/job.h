#pragma once
#include <deque>
#include <mutex>
#include <vector>
#include <memory>
#include "job_define.h"

namespace cloud
{
enum class JobState
{
    Setup,
    Started,
    Pending,
    Processing,
    Syspended
};

struct Job
{
    JobFunc task;
    JobState state;
    JobContext *context;
    uint32_t group_id;
    uint32_t group_job_offset;
    uint32_t group_job_end;
    uint32_t shared_memory_size;

    void Execute();
};

class JobQueue
{
  public:
    void PushBack(const Job &job);
    bool PopFront(Job &job);

  private:
    std::deque<Job> queue;
    std::mutex queue_mutex;
};

} // namespace cloud