#pragma once
#include <cstdint>
#include <functional>
#include "job_define.h"
#include "job.h"

namespace cloud
{
class WorkerThreads;
class JobSystem final
{
  public:
    JobSystem(uint32_t max_thread_count = 0);
    ~JobSystem();

    void Shutdown();

    void Wait(const JobContext &context);

    bool IsActive() const;
    bool IsBusy(const JobContext &context) const;
    uint32_t get_num_core() const;

    Job *create(Job *parent, JobFunc sub_task);
    Job *create_parent_job(Job *parent);
    void run(Job *job);

  protected:
    uint32_t dispatch_group_count(uint32_t job_count,
                                  uint32_t group_size) const;
    uint32_t calc_core_num(uint32_t core_num) const;

  private:
    std::unique_ptr<WorkerThreads> workers{nullptr};
};
} // namespace cloud