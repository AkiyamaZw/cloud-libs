#include "job_builder.h"
#include "job_system.h"

namespace cloud::js
{
JobBuilder::JobBuilder(JobSystem &js, JobPriority property)
    : js_(js)
    , priority_(property)
    , wait_counter_(js)
    , accumulate_counter_(js)
{
}

JobBuilder::~JobBuilder() { wait_counter_.finish_submit_job(); }

Counter JobBuilder::extract_wait_counter()
{

    wait_counter_.finish_submit_job();
    return accumulate_counter_;
}

void JobBuilder::dispatch(const std::string &name, JobFunc func)
{
    assert(!wait_counter_.finished());
    js_.create_job(name,
                   func,
                   priority_,
                   wait_counter_.get_entry(),
                   accumulate_counter_.get_entry());
    js_.try_signal(wait_counter_.get_entry());
}

void JobBuilder::dispatch_fence_explicitly()
{
    wait_counter_.finish_submit_job();
    wait_counter_ = accumulate_counter_;
    accumulate_counter_ = Counter(js_);
    disptach_empty_job();
}

void JobBuilder::dispatch_wait(const Counter &counter)
{
    assert(wait_counter_.get_cnt() == 0);
    wait_counter_ = counter;
    disptach_empty_job();
}

void JobBuilder::disptach_empty_job() { dispatch("empty job", nullptr); }

} // namespace cloud::js