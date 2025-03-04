#include "worker_threads.h"
#include "job.h"
#include <cassert>
#include <stdlib.h>

namespace cloud
{
WorkerThreads::WorkerThreads(uint32_t max_thread_count,
                             uint32_t max_adopt_thread_count,
                             std::function<bool(Worker &)> callback)
    : num_thread_(max_thread_count)
    , callback_(callback)
{
    workers_.resize(num_thread_ + max_adopt_thread_count);
    for (int i = 0; i < workers_.size(); ++i)
    {
        auto &worker = workers_[i];
        if (i < max_thread_count)
            worker.worker =
                std::thread(&WorkerThreads::worker_loop, this, &worker);
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

void WorkerThreads::worker_loop(Worker *worker)
{

    std::lock_guard<std::mutex> lock(map_locker_);
    auto check = worker_map_.emplace(std::this_thread::get_id(), worker).second;
    assert(check);
    while (alive_.load())
    {
        if (!callback_(*worker))
        {
            std::unique_lock<std::mutex> lock(wake_mutex);
            wake_condition.wait(lock);
        }
    }
}

Worker *WorkerThreads::get_worker()
{
    std::lock_guard<std::mutex> lock(map_locker_);
    auto iter = worker_map_.find(std::this_thread::get_id());
    assert(iter->second);
    return iter->second;
}

void WorkerThreads::try_wake_up(int32_t active_job)
{
    std::lock_guard lock(wake_mutex);
    if (active_job == 1)
        wake_condition.notify_one();
    else
        wake_condition.notify_all();
}

Worker *WorkerThreads::random_select(Worker &from)
{
    uint16_t const adopt_thread_num =
        adopt_threads_num_.load(std::memory_order_relaxed);
    uint16_t const thread_count = thread_count + adopt_thread_num;
    Worker *worker = nullptr;

    if (thread_count >= 2)
    {
        // todo random select
    }
    return worker;
}

} // namespace cloud