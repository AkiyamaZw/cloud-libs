#include "job.h"
namespace cloud
{
Job::Job() {}

Job::~Job() {}

Job::Job(const Job &rhs)
{
    task = rhs.task;
    state = rhs.state;
    context = rhs.context;
    running_job_count_.store(rhs.running_job_count_.load());
}

Job &Job::operator=(const Job &rhs)
{
    task = rhs.task;
    state = rhs.state;
    context = rhs.context;
    running_job_count_.store(rhs.running_job_count_.load());
    return *this;
}

void Job::Execute()
{
    JobArgs args;

    task(args);

    context->counter.fetch_sub(1);
}

} // namespace cloud
