#include "job_system.h"
#include <cassert>
#include "worker_threads.h"
#include "counter.h"
#include "job_counter_entry.h"
#include "job_system_extension.h"
#include "job.h"

namespace cloud::js
{

JobSystem::JobSystem(uint32_t max_thread_count, uint32_t max_adopt_thread)
{
    counter_pool_ = std::make_unique<CounterPool>();
    entry_pool_ = std::make_unique<JobWaitListEntryPool>();
    queue_proxy_ = std::make_unique<JobQueueProxy>(this);

    workers_ = new WorkerThreads();
    workers_->init(
        calc_core_num(max_thread_count),
        max_adopt_thread,
        std::bind(&JobSystem::execute_job, this, std::placeholders::_1));
}

JobSystem::~JobSystem()
{
    shutdown();
    delete workers_;
    entry_pool_.reset();
    counter_pool_.reset();
}

void JobSystem::shutdown()
{
    if (!is_active())
    {
        return;
    }

    workers_->toggle_alive(false);
}

bool JobSystem::is_active() const { return workers_->get_alive(); }

uint32_t JobSystem::get_num_core() const
{
    return workers_ ? workers_->get_thread_num() : 0u;
}

bool JobSystem::has_active_job() const
{
    return queue_proxy_->get_active_jobs() > 0;
}

void JobSystem::adopt() { workers_->attach(); }

void JobSystem::emancipate() { workers_->detach(); }

JobWaitEntry *JobSystem::create_job(const std::string &name,
                                    JobFunc task,
                                    JobPriority priority,
                                    JobCounterEntry *wait_counter,
                                    JobCounterEntry *acc_counter)
{
    JobWaitEntry *entry = entry_pool_->get();
    entry->init(name, task, acc_counter, priority);
    wait_counter->add_dep_jobs(entry);
    acc_counter->accumulate();
    return entry;
}

void JobSystem::try_signal(JobCounterEntry *counter)
{
    if (counter->get_waits() != 0)
        return;
    // signal other counters
    counter->on_counter_signal();
    {
        std::lock_guard lock(counter->dep_counter_lock_);
        while (!counter->wait_counter_list_.empty())
        {
            auto entry = counter->wait_counter_list_.front();
            counter->wait_counter_list_.pop_front();
            entry->signal();
        }
    }
    try_dispatch_job(counter);
}

JobCounterEntry *JobSystem::create_entry_counter()
{
    auto counter = counter_pool_->get();
    counter->init(
        [this](JobCounterEntry *counter) { this->try_signal(counter); });
    return counter;
}

void JobSystem::try_dispatch_job(JobCounterEntry *counter)
{
    assert(counter->get_waits() == 0);
    {
        std::lock_guard lock(counter->dep_jobs_lock_);
        auto worker = workers_->get_worker();

        JobWaitEntry *pkt = nullptr;
        while (!counter->wait_job_list_.empty())
        {
            pkt = counter->wait_job_list_.front();
            counter->wait_job_list_.pop_front();

            if (!pkt->job_.is_empty())
            {
                queue_proxy_->push_job(worker->job_queue, pkt);
            }
            else
            {
                // cause empty job, goto after_job_execute directly.
                after_job_execute(pkt);
            }
        }
    }
}

void JobSystem::spin_wait(const Counter &counter)
{
    // just work like spin lock
    while (counter.get_entry()->get_waits() != 0)
    {
        /* workers_->try_wake_up_all();*/
    }
}

uint32_t JobSystem::calc_core_num(uint32_t max_core_num)
{

    max_core_num = std::max(1u, max_core_num);
    uint32_t num_core = std::thread::hardware_concurrency();
    return std::clamp(num_core - 1, 1u, max_core_num);
}

bool JobSystem::execute_job(Worker &worker)
{
    // 1. get job from this_thread
    auto *job_pkt = queue_proxy_->pop_job(worker.job_queue);
    // 2. if not get job from other threads
    if (job_pkt == nullptr)
    {
        job_pkt = queue_proxy_->steal(worker);
    }
    // 3. execute job
    bool valid_job = job_pkt != nullptr;
    if (valid_job)
    {
        // 1. check job is running?
        JobArgs args{};
        job_pkt->job_.execute(args);
        after_job_execute(job_pkt);
    }
    return valid_job;
}

void JobSystem::after_job_execute(JobWaitEntry *job)
{
    job->accumulate_counter_->signal();
    entry_pool_->release(job);
}

} // namespace cloud::js