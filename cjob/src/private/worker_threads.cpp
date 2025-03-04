#include "worker_threads.h"
#include "job.h"
#include <cassert>

namespace cloud
{
WorkerThreads::WorkerThreads(uint32_t max_thread_count)
    : num_thread_(max_thread_count)
{
    workers_.resize(num_thread_);
    for (int i = 0; i < num_thread_; ++i)
    {
        auto &worker = workers_[i];
        worker.worker = std::thread(&WorkerThreads::worker_loop, this, &worker);
    }
}

WorkerThreads::~WorkerThreads()
{
    toggle_alive(false);
    if (alive_.load())
        num_thread_ = 0;
    worker_map_.clear();
    workers_.clear();
}

void WorkerThreads::toggle_alive(bool value)
{
    if (!alive_.load())
        return;
    alive_.store(false);

    if (value)
        return;
    // make exit
    bool wake_loop = true;
    std::thread waker([&]() {
        while (wake_loop)
        {
            wake_condition.notify_all();
        }
    });

    for (auto &th : workers_)
    {
        th.worker.join();
    }

    wake_loop = false;
    waker.join();
}

void WorkerThreads::put(Worker &worker, Job *job)
{
    // single thread
    if (num_thread_ < 1)
    {
        job->Execute();
    }
    else
    {
        worker.job_queue.push(*job);
        auto value = active_jobs_.fetch_add(1, std::memory_order_relaxed);
        std::lock_guard lock(wake_mutex);
        if (value == 1)
        {
            wake_condition.notify_one();
        }
        else
        {
            wake_condition.notify_all();
        }
    }
}

bool WorkerThreads::pop(JobQueue &queue, Job &job)
{
    active_jobs_.fetch_sub(1, std::memory_order_relaxed);
    return queue.pop(job);
}

void WorkerThreads::worker_loop(Worker *worker)
{

    std::lock_guard<std::mutex> lock(map_locker_);
    auto check = worker_map_.emplace(std::this_thread::get_id(), worker).second;
    assert(check);
    while (alive_.load())
    {
        if (!execute_job(*worker))
        {
            std::unique_lock<std::mutex> lock(wake_mutex);
            wake_condition.wait(lock);
        }
    }
}

bool WorkerThreads::execute_job(Worker &worker)
{
    Job job;
    bool succ = pop(*worker.job_queue, job);
    if (!succ)
    {
        // steal
    }
    if (succ)
    {
        if (job.task != nullptr)
        {
            job.Execute();
        }
    }
    return succ;
}

WorkerThreads::Worker &WorkerThreads::get_worker()
{
    std::lock_guard<std::mutex> lock(map_locker_);
    auto iter = worker_map_.find(std::this_thread::get_id());
    assert(iter->second);
    return *iter->second;
}

} // namespace cloud