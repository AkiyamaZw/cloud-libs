#pragma once
#include <cstdint>
#include <functional>
#include "job_define.h"

namespace cloud
{

class JobSystem
{
  public:
    JobSystem();
    ~JobSystem();

    void Active(uint32_t max_thread_count = 0);
    void Shutdown();
    void Wait(const JobContext &context);
    void Execute(JobContext &context, const std::function<void(JobArgs)> &task);

    bool IsActive();
    bool IsBusy();

  private:
};
} // namespace cloud