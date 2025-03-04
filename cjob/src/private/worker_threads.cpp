#include "worker_threads.h"
#include "job.h"
#include <cassert>
#include <stdlib.h>

namespace cloud
{
WorkerThreads::WorkerThreads() {}

void WorkerThreads::init(uint32_t max_thread_count,
                         uint32_t max_adopt_thread_count,
                         std::function<bool(Worker &)> callback)
{
    num_thread_ = max_thread_count;
    callback_ = callback;
    rand_engine_ = std::mt19937(std::random_device{}());

    workers_ = std::vector<Worker>(num_thread_ + max_adopt_thread_count);
    rand_generator_ = std::uniform_int_distribution<int>(
        0, num_thread_ + max_adopt_thread_count);

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
    if (value)
        return;
    alive_.store(false);

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

        if (th.worker.joinable())
            th.worker.join();
    }

    wake_loop = false;
    waker.join();
}

void WorkerThreads::worker_loop(Worker *worker)
{
    {
        std::lock_guard<std::mutex> lock(map_locker_);
        auto check =
            worker_map_.emplace(std::this_thread::get_id(), worker).second;
        assert(check);
    }
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

void WorkerThreads::attach()
{
    const auto thread_id = std::this_thread::get_id();

    std::unique_lock lock(map_locker_);
    auto iter = worker_map_.find(thread_id);
    auto worker = iter == worker_map_.end() ? nullptr : iter->second;
    lock.unlock();

    if (worker)
    {
        return;
    }
    uint16_t adopt = adopt_threads_num_.fetch_add(1, std::memory_order_relaxed);
    size_t index = num_thread_ + adopt;
    assert(index < workers_.size());

    lock.lock();
    worker_map_[thread_id] = &workers_[index];
}

void WorkerThreads::detach()
{
    const auto thread_id = std::this_thread::get_id();
    std::lock_guard locker(map_locker_);
    auto iter = worker_map_.find(thread_id);
    auto worker = iter == worker_map_.end() ? nullptr : iter->second;
    if (worker != nullptr)
        worker_map_.erase(iter);
    adopt_threads_num_.fetch_sub(1, std::memory_order_relaxed);
}

void WorkerThreads::try_wake_up(int32_t active_job)
{
    std::lock_guard lock(wake_mutex);
    if (active_job == 1)
        wake_condition.notify_one();
    else
        wake_condition.notify_all();
}

void WorkerThreads::try_wake_up_all()
{
    std::lock_guard lock(wake_mutex);
    wake_condition.notify_all();
}

Worker *WorkerThreads::random_select(Worker &from)
{
    uint16_t const adopt_thread_num =
        adopt_threads_num_.load(std::memory_order_relaxed);
    uint16_t const thread_count = num_thread_ + adopt_thread_num;
    Worker *worker = nullptr;

    if (thread_count >= 2)
    {
        do
        {
            uint16_t index =
                uint16_t(rand_generator_(rand_engine_) % thread_count);
            assert(index < thread_count);
            worker = &workers_[index];
        } while (worker == &from);
    }
    return worker;
}

void WorkerThreads::wait(Job *job)
{
    std::unique_lock lock(wake_mutex);
    wake_condition.wait(lock);
}

} // namespace cloud