#include "job_system.h"
#include <cassert>
#include <algorithm>
#include "worker_threads.h"
#include "counter.h"
#include "job_counter_entry.h"
#include <iostream>

namespace cloud::js
{
JobSystemExtension::JobSystemExtension(JobSystem *js)
    : js_(js)
{
    assert(js_ != nullptr);
}

JobSystemExtension::~JobSystemExtension() { js_ = nullptr; }

JobWaitEntry *JobQueueProxy::pop_job(JobQueue &queue)
{
    JobWaitEntry *entry = nullptr;

    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.pop();

    entry = !index ? nullptr : js_->entry_pool_->at(index - 1);
    if (entry == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            js_->workers_->try_wake_up(old_jobs);
        }
    }
    return entry;
}

void JobQueueProxy::push_job(JobQueue &queue, JobWaitEntry *job_pack)
{
    assert(job_pack != nullptr);
    // this means left 0 to invalid default.
    queue.push(job_pack->get_index() + 1);
    auto old_value = active_jobs_.fetch_add(1, std::memory_order_relaxed);
    if (old_value >= 0)
    {
        js_->workers_->try_wake_up(old_value + 1);
    }
}

JobWaitEntry *JobQueueProxy::steal_job(JobQueue &queue)
{
    JobWaitEntry *job_pkt{nullptr};
    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    auto index = queue.steal();
    job_pkt = !index ? nullptr : js_->entry_pool_->at(index - 1);
    if (job_pkt == nullptr)
    {
        auto old_jobs = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        if (old_jobs >= 0)
        {
            js_->workers_->try_wake_up(old_jobs);
        }
    }
    return job_pkt;
}

JobWaitEntry *JobQueueProxy::steal(Worker &worker)
{
    JobWaitEntry *job_pkt{nullptr};
    do
    {
        assert(js_->workers_ != nullptr);
        auto target_worker = js_->workers_->random_select(worker);
        if (target_worker)
        {
            job_pkt = steal_job(target_worker->job_queue);
        }
    } while (job_pkt == nullptr && js_->has_active_job());
    return job_pkt;
}

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

void JobSystem::commit_job(JobWaitEntry *job_pkt)
{
    auto worker = workers_->get_worker();
    if (job_pkt->job != nullptr && !job_pkt->job->is_empty())
    {
        queue_proxy_->push_job(worker->job_queue, job_pkt);
    }
    else
    {
        // cause empty job, no need to push_job
        job_pkt->accumulate_counter->signal();
        try_signal(job_pkt->accumulate_counter);
    }
}

void JobSystem::try_signal(JobCounterEntry *counter)
{
    if (counter->get_waits() != 0)
        return;
    // signal other counters
    {
        std::lock_guard lock(counter->dep_counter_lock_);
        while (!counter->wait_counter_list_.empty())
        {
            auto entry = counter->wait_counter_list_.front();
            counter->wait_counter_list_.pop_front();
            entry->signal();
            try_dispatch(entry);
        }
    }
    try_dispatch(counter);
}

JobWaitEntry *JobSystem::create_job(const std::string &name,
                                    JobFunc task,
                                    JobCounterEntry *wait_counter,
                                    JobCounterEntry *acc_counter)
{
    JobWaitEntry *entry = entry_pool_->get();
    entry->job->init(name, task);
    entry->accumulate_counter = acc_counter;
    wait_counter->add_dep_jobs(entry);
    acc_counter->accumulate();
    return entry;
}

JobCounterEntry *JobSystem::create_entry_counter()
{
    auto counter = counter_pool_->get();
    counter->init();
    return counter;
}

void JobSystem::release_counter(JobCounterEntry *counter)
{
    if (counter->ready_to_release())
    {
        std::cout << counter_pool_->debug_info() << std::endl;
        counter->on_counter_destroyed();
        counter_pool_->release(counter);
    }
}

void JobSystem::try_dispatch(JobCounterEntry *counter)
{
    if (counter->get_waits() != 0)
        return;

    counter->on_counter_signal();
    {
        std::lock_guard lock(counter->dep_jobs_lock_);
        while (!counter->wait_job_list_.empty())
        {
            auto pkt = counter->wait_job_list_.front();
            counter->wait_job_list_.pop_front();
            commit_job(pkt);
        }
    }
    release_counter(counter);
}

void JobSystem::spin_wait(JobCounterEntry *counter)
{
    // just work like spin lock
    while (counter->get_waits() != 0)
    {
        /* workers_->try_wake_up_all();*/
    }
}

void JobSystem::spin_wait(const Counter &counter)
{
    spin_wait(counter.get_entry());
}

uint32_t JobSystem::dispatch_group_count(uint32_t job_count,
                                         uint32_t group_size) const
{
    return (job_count + group_size - 1) / group_size;
}

uint32_t JobSystem::calc_core_num(uint32_t max_core_num) const
{

    max_core_num = std::max(1u, max_core_num);
    uint32_t num_core = std::thread::hardware_concurrency();
    return std::clamp(num_core - 1, 1u, max_core_num);
}

void JobSystem::release_job(JobWaitEntry *job_pkt)
{
    entry_pool_->release(job_pkt);
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
        job_pkt->job->execute(args);

        job_pkt->accumulate_counter->signal();
        try_signal(job_pkt->accumulate_counter);
        release_job(job_pkt);
    }
    return valid_job;
}

} // namespace cloud::js