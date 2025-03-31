#pragma once
#include "job_define.h"
#include "resource_pool.h"

namespace cloud
{

class Job
{
  public:
    enum class State : uint8_t
    {
        Empty,
        Setup,
        Running,
        Finished,
    };

  public:
    ~Job();
    Job(const Job &job);
    Job &operator=(const Job &job);
    // move operator not support!
    Job(Job &&) = delete;
    Job &operator=(Job &&) = delete;

    void init(const std::string &name, JobFunc task);
    void reset();

    std::string get_name() const { return name_; }
    State get_state() const { return state_; }

    bool is_completed() const { return state_.load() == State::Finished; }
    bool is_empty() const { return state_.load() == State::Empty; }
    void execute(JobArgs &args);

    friend class JobWaitEntry;

  private:
    Job() = default;
    JobFunc task_;
    std::atomic<State> state_{State::Setup};
    std::string name_;
};

class JobCounterEntry;

struct JobWaitEntry : public internal::IPoolableObject
{

    JobWaitEntry() { job = new Job(); }
    ~JobWaitEntry() { delete job; }

    void reset() override
    {
        accumulate_counter = nullptr;
        job->reset();
    }

    JobCounterEntry *accumulate_counter{nullptr};
    Job *job{nullptr};
};

} // namespace cloud