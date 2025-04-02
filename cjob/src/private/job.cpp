#include "job.h"
#include "job_counter_entry.h"

namespace cloud::js
{

Job::~Job() {}

Job::Job(const Job &rhs) { task_ = rhs.task_; }

Job &Job::operator=(const Job &rhs)
{
    task_ = rhs.task_;
    state_.store(rhs.state_.load());
    return *this;
}

void Job::init(const std::string &name, JobFunc task)
{
    name_ = name;
    task_ = task;
    if (task == nullptr)
    {
        state_.store(State::Empty);
    }
    else
    {
        task_ = task;
        state_.store(State::Setup);
    }
}

void Job::execute(JobArgs &args)
{
    state_.store(State::Running);
    task_(args);
    state_.store(State::Finished);
}

void Job::reset()
{
    name_ = "Unsetup";
    task_ = nullptr;
    state_.store(State::Empty);
}

void JobWaitEntry::init(const std::string &name,
                        JobFunc task,
                        JobCounterEntry *acc_counter)
{
    job_.init(name, task);
    accumulate_counter_ = acc_counter;
    accumulate_counter_->add_ref();
}

void JobWaitEntry::reset()
{
    JobCounterEntry::sub_ref_and_try_release(accumulate_counter_);
    accumulate_counter_ = nullptr;
    job_.reset();
}
} // namespace cloud::js
