#pragma once
#include "job_define.h"

namespace cloud::js
{
class JobSystem;
class WorkerThreads;
/* */
class JobSystemProxy
{
  public:
    using Super = JobSystemProxy;
    JobSystemProxy(JobSystem *js);
    virtual ~JobSystemProxy();
    JobWaitListEntryPool &entry_pool();
    CounterPool &counter_pool();
    WorkerThreads &workers();

  protected:
    JobSystem *js_;
};

} // namespace cloud::js