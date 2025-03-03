#include "worker_threads.h"
#include "job.h"
namespace cloud
{
WorkerThreads::WorkerThreads(uint32_t max_thread_count)
    : num_thread_(max_thread_count)
{
    for (int i = 0; i < num_thread_; ++i)
        job_queues_per_thread.push_back(std::make_shared<JobQueue>());
    threads.reserve(num_thread_);

    for (uint32_t thread_id = 0; thread_id < num_thread_; ++thread_id)
    {
        std::thread &worker = threads.emplace_back([this, thread_id]() {
            while (alive_.load())
            {
                Work(thread_id);
                std::unique_lock<std::mutex> lock(wake_mutex);
                wake_condition.wait(lock);
            }
        });
    }
}

WorkerThreads::~WorkerThreads()
{
    job_queues_per_thread.clear();
    threads.clear();
    num_thread_ = 0;
}

void WorkerThreads::Work(uint32_t starting_queue)
{
    Job job;
    for (uint32_t i = 0; i < num_thread_; ++i)
    {
        auto job_queue = job_queues_per_thread[starting_queue % num_thread_];
        while (job_queue->PopFront(job))
        {
            job.Execute();
        }
        starting_queue += 1;
    }
}

void WorkerThreads::toggle_alive(bool value)
{
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

    for (auto &thread : threads)
    {
        thread.join();
    }

    wake_loop = false;
    waker.join();
}
} // namespace cloud