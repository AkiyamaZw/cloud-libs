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

class JobPool;
struct Job
{
  public:
    Job();
    ~Job();
    Job(const Job &job);
    Job &operator=(const Job &job);

    JobFunc task;
    JobState state{JobState::Setup};
    JobContext *context{nullptr};

    // a count flag for dependency. means how much sub job in this job.
    std::atomic<uint16_t> running_job_count_{1};

    std::atomic<uint16_t> ref_count = {1};

    static Job *create() { return new Job(); }

    void Execute();

    bool is_completed() const
    {
        running_job_count_.load(std::memory_order_acquire) <= 0;
    }

  private:
    JobPool *job_pool_;
};

class JobQueue
{
  public:
    void PushBack(Job &job);
    bool PopFront(Job &job);

  private:
    std::deque<Job> queue;
    std::mutex queue_mutex;
};

// class JobPool
// {
//   public:
//     JobPool();
//     ~JobPool();

//     Job *get();
//     void release(Job *job);

//   private:
//     std::deque<Job *> free_list_;
//     std::vector<Job *> in_use_list_;
// };

} // namespace cloud