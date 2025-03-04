#pragma once
#include <vector>
#include <cstdint>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <random>
#include "job_define.h"

namespace cloud
{
struct Job;

struct Worker
{
    JobQueue job_queue;
    std::thread worker;
};

class WorkerThreads
{
  public:
    WorkerThreads(uint32_t max_thread_count,
                  uint32_t max_adopt_thread_count,
                  std::function<bool(Worker &)> callback);
    ~WorkerThreads();

    std::vector<Worker> workers_;
    std::unordered_map<std::thread::id, Worker *> worker_map_;
    std::mutex map_locker_;

    void toggle_alive(bool value);
    bool get_alive() const { return alive_.load(); }
    Worker *get_worker();

    void try_wake_up(int32_t active_job);
    Worker *random_select(Worker &from);

  private:
    void worker_loop(Worker *worker);

  protected:
    std::atomic_bool alive_{true};
    std::function<bool(Worker &)> callback_;
    uint32_t num_thread_{0};
    std::condition_variable wake_condition;
    std::mutex wake_mutex;
    // have queue but is not a processor
    std::atomic<uint16_t> adopt_threads_num_{0};

    std::default_random_engine rand_generator;
};
} // namespace cloud