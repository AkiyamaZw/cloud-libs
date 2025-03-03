#pragma once
#include <vector>
#include <cstdint>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

namespace cloud
{
struct JobQueue;
struct Job;

class WorkerThreads
{
  protected:
    struct Worker
    {
        std::unique_ptr<JobQueue> job_queue;
        std::thread worker;
    };

  public:
    WorkerThreads(uint32_t max_thread_count);
    ~WorkerThreads();
    uint32_t num_thread_{0};
    std::atomic<uint32_t> next_queue{0};
    std::condition_variable wake_condition;
    std::mutex wake_mutex;

    std::vector<Worker> workers_;
    std::unordered_map<std::thread::id, Worker *> worker_map_;
    std::mutex map_locker_;

    void toggle_alive(bool value);
    bool get_alive() const { return alive_.load(); }

    void put(Worker &worker, Job *job);
    bool pop(JobQueue &queue, Job &job);
    Worker &get_worker();

  private:
    void worker_loop(Worker *worker);
    bool execute_job(Worker &worker);

  protected:
    std::atomic_bool alive_{true};
    std::atomic<int32_t> active_jobs_{0};
};
} // namespace cloud