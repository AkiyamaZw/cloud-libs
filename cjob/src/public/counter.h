#pragma once
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <string>
#include <vector>
#include <deque>
#include <atomic>

namespace cloud
{
using CJobFunc = std::function<void()>;
struct JobWaitListEntry;

struct JobInner
{
    static JobInner *create(const std::string &name, CJobFunc func)
    {
        JobInner *task = new JobInner();
        static uint32_t job_id = 0;
        task->id_ = ++job_id;
        task->name_ = name;
        task->ref.fetch_add(1);
        return task;
    }
    static void release(JobInner *job)
    {
        auto ref_last = job->ref.fetch_sub(1);
        if (ref_last == 1)
        {
            delete job;
            job = nullptr;
        }
    }

    CJobFunc task;
    std::atomic_uint16_t ref{0};
    uint32_t id_{0};
    std::string name_;
};

struct CJob
{
    CJob(const std::string &name, CJobFunc task)
    {
        job = JobInner::create(name, task);
    };
    CJob() = default;
    ~CJob() { JobInner::release(job); }
    CJob(const CJob &rhs);
    CJob(CJob &&rhs) = delete;

    CJob &operator=(const CJob &rhs)
    {
        job = rhs.job;
        job->ref.store(rhs.job->ref.load() + 1);
        return *this;
    }

    CJob &operator=(CJob &&rhs) = delete;

    uint32_t get_id() const { return job != nullptr ? job->id_ : 0; }
    std::string get_name() const
    {
        return job != nullptr ? job->name_ : "invalid_job";
    }
    JobInner *job{nullptr};
};

class JobCounterEntry
{
    friend class Counter;

  private:
    JobCounterEntry() = default;
    ~JobCounterEntry();
    JobCounterEntry(const JobCounterEntry &) = delete;
    JobCounterEntry(JobCounterEntry &&) = delete;
    JobCounterEntry &operator=(const JobCounterEntry &) = delete;
    JobCounterEntry &operator=(JobCounterEntry &&) = delete;
    static JobCounterEntry *create()
    {
        JobCounterEntry *entry = new JobCounterEntry();
        static uint32_t id = 0;
        entry->instance_id_ = ++id;
        entry->ref_.fetch_add(1, std::memory_order_relaxed);
        return entry;
    };

    static void release(JobCounterEntry *entry)
    {
        if (entry != nullptr)
        {
            auto x = entry->ref_.fetch_sub(1);
            if (x == 1)
            {
                delete entry;
                entry = nullptr;
            }
        }
    }

    void add_dep_jobs(CJob &job, Counter &counter);
    std::atomic_uint16_t cnt_{0};
    std::atomic_uint16_t ref_{0};
    // std::vector<CJob> dep_jobs_{};
    std::mutex dep_jobs_lock_;

    uint32_t instance_id_{0};
    JobWaitListEntry *wait_list_head_{nullptr};
};

class Counter
{
  public:
    Counter();
    ~Counter();
    Counter(const Counter &rhs);
    Counter(Counter &&rhs) noexcept;
    Counter &operator=(const Counter &rhs);
    Counter &operator=(Counter &&rhs);
    Counter &operator+=(const Counter &rhs);
    bool operator==(const Counter &rhs) const;

    constexpr bool is_valid() const { return entry_ != nullptr; }
    uint32_t get_cnt() const { return is_valid() ? entry_->cnt_.load() : 0; }
    uint32_t get_ref() const { return is_valid() ? entry_->ref_.load() : 0; }
    void add_dep_task(CJob &job, Counter &accu_counter) const;
    void add(uint32_t i = 1) { entry_->cnt_.fetch_add(1); }
    void sub(uint32_t i = 1) { entry_->cnt_.fetch_sub(1); }
    void set_cnt(uint32_t i) { entry_->cnt_.store(i); }
    uint32_t get_id() const { return entry_->instance_id_; }
    const JobWaitListEntry *get_jobs() const { return entry_->wait_list_head_; }

  private:
    JobCounterEntry *entry_{nullptr};
};

struct JobWaitListEntry
{
    JobWaitListEntry *next{nullptr};
    Counter accumulate_counter;
    CJob cjob;
};

class RunContext;
class JobBuilder
{
  public:
    using JobFunc = std::function<void()>;
    JobBuilder();
    ~JobBuilder();
    // new job to dispatch
    void dispatch(const std::string &name, JobFunc func);
    // export wait 'cnt' of wait count, and turn wait counter's state to
    // invalid
    Counter extract_wait_counter();
    // create a fence among builder's scope
    void dispatch_fence_explicitly();
    // connect jobbuilder by using counter. always used after
    void dispatch_wait(const Counter &counter);

  private:
    Counter wait_counter_;
    Counter accumulate_counter_;
};

class RunContext
{
  public:
    static RunContext *get_context()
    {
        static RunContext context;
        return &context;
    }
    void push_wait_jobs(Counter &counter);

    void export_grapviz(const std::string &path);

  private:
    std::string counter_grapviz(const Counter &counter);
    std::string job_graphviz(const CJob &job) const;

  private:
    std::unordered_map<uint32_t, Counter> wait_counter_queue_;
    std::unordered_map<std::thread::id, Counter> cur_counters_;
    std::mutex cur_counters_map_mutex_;
};
} // namespace cloud