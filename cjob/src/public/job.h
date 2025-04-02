#pragma once
#include "job_define.h"
#include "resource_pool.h"

namespace cloud::js
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

class JobWaitEntry : public internal::IPoolableObject<JobWaitEntry>
{
  public:
    ~JobWaitEntry() {}
    JobWaitEntry(const JobWaitEntry &) = delete;
    JobWaitEntry(JobWaitEntry &&) = delete;
    JobWaitEntry &operator=(const JobWaitEntry &) = delete;
    JobWaitEntry &operator=(JobWaitEntry &&) = delete;

    void init(const std::string &name,
              JobFunc task,
              JobCounterEntry *acc_counter);

    void reset() override;

    JobCounterEntry *accumulate_counter_{nullptr};
    Job job_;

  protected:
    friend class internal::ResourcePool<JobWaitEntry>;
    JobWaitEntry() {}
};

} // namespace cloud::js