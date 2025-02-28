#pragma once
#include <cstdint>
#include <functional>
#include "job_define.h"
#include "job.h"

namespace cloud
{

class JobSystem final
{
  public:
    JobSystem(uint32_t max_thread_count = 0);
    ~JobSystem();

    void Shutdown();

    void Wait(const JobContext &context);
    void Execute(JobContext &context, const std::function<void(JobArgs)> &task);
    void Dispatch(JobContext &context,
                  uint32_t job_count,
                  uint32_t group_size,
                  const std::function<void(JobArgs)> &task,
                  size_t shared_memory_size);

    bool IsActive() const;
    bool IsBusy(const JobContext &context) const;
    uint32_t GetNumCore() const { return num_core; }

  protected:
    uint32_t DispatchGroupCount(uint32_t job_count, uint32_t group_size) const;
    uint32_t calc_core_num(uint32_t core_num) const;

  private:
    uint32_t num_core{0};
    std::unique_ptr<WorkerThreads> workers{nullptr};
    std::atomic_bool alive{true};
};
} // namespace cloud