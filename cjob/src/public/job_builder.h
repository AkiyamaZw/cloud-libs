#pragma once
#include "counter.h"

namespace cloud::js
{
class JobSystem;
class JobBuilder
{
  public:
    JobBuilder(JobSystem &js);
    ~JobBuilder();
    // new job to dispatch
    void dispatch(const std::string &name, JobFunc func);
    // export wait 'cnt' of wait count, and turn wait counter's state to
    // invalid
    Counter extract_wait_counter();
    // create a fence among builder's scope
    void dispatch_fence_explicitly();
    // connect jobbuilder by using counter. always used after
    void dispatch_wait(const Counter &counter);

  private:
    void disptach_empty_job();

  private:
    JobSystem &js_;
    Counter wait_counter_;
    Counter accumulate_counter_;
};
} // namespace cloud::js