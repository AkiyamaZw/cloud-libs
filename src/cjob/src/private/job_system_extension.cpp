#include "job_system_extension.h"
#include "job_system.h"
#include "job_counter_entry.h"
#include "worker_threads.h"
#include "resource_pool.h"
#include "job.h"

namespace cloud::js
{
JobSystemProxy::JobSystemProxy(JobSystem *js)
    : js_(js)
{
    assert(js_ != nullptr);
}

JobSystemProxy::~JobSystemProxy() { js_ = nullptr; }

JobWaitListEntryPool &JobSystemProxy::entry_pool() { return *js_->entry_pool_; }

CounterPool &JobSystemProxy::counter_pool() { return *js_->counter_pool_; }

WorkerThreads &JobSystemProxy::workers() { return *js_->workers_; }

} // namespace cloud::js