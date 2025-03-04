#pragma once
#include <deque>
#include <mutex>
#include <vector>
#include <memory>
#include "job_define.h"
#include "stealable_queue.h"

namespace cloud
{

class JobPool;

struct JobHandle
{
    static constexpr uint32_t INVALID_HANDLE_INDEX = uint32_t(-1);
    uint32_t index{INVALID_HANDLE_INDEX};
    bool is_valid() const { return index != INVALID_HANDLE_INDEX; }
    operator uint32_t() { return index; }
};

class Job
{
  public:
    ~Job();
    Job(const Job &job);
    Job &operator=(const Job &job);
    Job(Job &&) = delete;
    Job &operator=(Job &&) = delete;

    JobFunc task;
    // handle use in outer
    JobHandle handle_;
    JobHandle parent_;

    // a count flag for dependency. means how much sub job in this job.
    std::atomic<uint16_t> running_job_count_{1};
    static constexpr JobHandle INVALID_HANDLE{};

    bool is_completed() const
    {
        return running_job_count_.load(std::memory_order_acquire) <= 0u;
    }

  protected:
    friend class JobPool;
    Job() = default;
    void reset();
};

class JobPool
{
  public:
    JobPool() = default;
    ~JobPool();

    Job *get();
    Job *at(uint32_t index);
    Job *at(const JobHandle &handle);
    void release(Job *job);

  protected:
    Job *create();

  private:
    std::mutex mutex_;
    std::vector<Job *> job_pool_;
    std::deque<uint32_t> free_list_;
};

} // namespace cloud