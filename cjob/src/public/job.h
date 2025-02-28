#pragma once
#include <functional>
#include <deque>
#include <mutex>
#include <vector>
#include <memory>
#include <thread>
#include <condition_variable>
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
    std::function<void(JobArgs)> task;
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

class WorkerThreads
{
  public:
    WorkerThreads(uint32_t max_thread_count);
    ~WorkerThreads();
    uint32_t num_thread_{0};
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<JobQueue>> job_queues_per_thread;
    std::atomic<uint32_t> next_queue{0};
    std::condition_variable wake_condition;
    std::mutex wake_mutex;

    void Work(uint32_t starting_queue);
};

} // namespace cloud