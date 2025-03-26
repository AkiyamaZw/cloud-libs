#include "job.h"
namespace cloud
{

Job::~Job() {}

Job::Job(const Job &rhs) { task_ = rhs.task_; }

Job &Job::operator=(const Job &rhs)
{
    task_ = rhs.task_;
    return *this;
}

void Job::init(const std::string &name, JobFunc task)
{
    name_ = name;
    task_ = task;
    if (task == nullptr)
    {
        state.store(State::Empty);
    }
    else
    {
        task_ = task;
        state.store(State::Setup);
    }
}

void Job::execute(JobArgs &args)
{
    state.store(State::Running);
    if (is_completed())
        return;
    task_(args);
    state.store(State::Finished);
}

void Job::reset()
{
    state.store(State::Empty);
    name_ = "Unsetup";
}
} // namespace cloud
