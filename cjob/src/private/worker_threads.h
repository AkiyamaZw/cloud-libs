#pragma once
#include <vector>
#include <cstdint>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace cloud
{
struct JobQueue;

class WorkerThreads
{
  public:
    WorkerThreads(uint32_t max_thread_count);
    ~WorkerThreads();
    uint32_t num_thread_{0};
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<JobQueue>> job_queues_per_thread;
    std::atomic<uint32_t> next_queue{0};
    std::condition_variable wake_condition;
    std::mutex wake_mutex;

    void Work(uint32_t starting_queue);

    void toggle_alive(bool value);
    bool get_alive() const { return alive_.load(); }

  protected:
    std::atomic_bool alive_{true};
};
} // namespace cloud