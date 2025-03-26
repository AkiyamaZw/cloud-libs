#pragma once
#include "job_define.h"
#include "stealable_queue.h"
#include "resource_pool.h"

namespace cloud
{

class Job : public internal::IPoolableObject
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
    Job() = default;
    ~Job();
    Job(const Job &job);
    Job &operator=(const Job &job);
    // move operator not support!
    Job(Job &&) = delete;
    Job &operator=(Job &&) = delete;

    void init(const std::string &name, JobFunc task);
    std::string get_name() const { return name_; }

    bool is_completed() const { return state.load() == State::Finished; }
    void execute(JobArgs &args);
    void reset() override;
    bool is_empty() const { return state.load() == State::Empty; }

  protected:
    friend class JobPool;

  private:
    JobFunc task_;
    std::atomic<State> state{State::Setup};
    std::string name_;
};

} // namespace cloud