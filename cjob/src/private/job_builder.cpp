#include "job_builder.h"
#include "job_system.h"

namespace cloud
{
JobBuilder::JobBuilder(JobSystem &js)
    : js_(js)
    , wait_counter_(nullptr)
    , accumulate_counter_(nullptr)
{
    wait_counter_.set_entry(js.create_entry_counter());
    accumulate_counter_.set_entry(js.create_entry_counter());
}

JobBuilder::~JobBuilder() { js_.release_counter(wait_counter_.get_entry()); }

Counter JobBuilder::extract_wait_counter()
{
    assert(accumulate_counter_.is_valid());

    wait_counter_.finish_submit_job();

    return accumulate_counter_;
}

void JobBuilder::dispatch(const std::string &name, JobFunc func)
{
    assert(wait_counter_.is_valid());
    js_.create_job_packet(
        name, func, wait_counter_.get_entry(), accumulate_counter_.get_entry());
    js_.try_signal(wait_counter_.get_entry());
}

void JobBuilder::dispatch_fence_explicitly()
{
    wait_counter_.finish_submit_job();
    wait_counter_ = accumulate_counter_;
    accumulate_counter_ = Counter::create(js_);
    accumulate_counter_.add();
}

void JobBuilder::dispatch_wait(const Counter &counter)
{
    assert(wait_counter_.get_cnt() == 0);
    wait_counter_ = counter;
    disptach_empty_job();
}

void JobBuilder::disptach_empty_job() { dispatch("empty job", nullptr); }

} // namespace cloud