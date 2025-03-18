#include "counter.h"
#include <cassert>

namespace cloud
{
void JobCounterEntry::add_dep_jobs(CJob *job)
{
    std::lock_guard lock(dep_jobs_lock_);
    dep_jobs_.insert(job);
}

void JobCounterEntry::add_dep_jobs(std::set<CJob *> jobs)
{
    std::lock_guard lock(dep_jobs_lock_);
    dep_jobs_.merge(jobs);
}

Counter::Counter() { entry_ = JobCounterEntry::create(); }

Counter::~Counter() { JobCounterEntry::release(entry_); }

Counter::Counter(const Counter &rhs)
{
    entry_ = rhs.entry_;
    entry_->ref_.fetch_add(1, std::memory_order_relaxed);
}

Counter::Counter(Counter &&rhs) noexcept
{
    entry_ = rhs.entry_;
    rhs.entry_ = nullptr;
}

Counter &Counter::operator=(const Counter &rhs)
{
    if (&rhs == this || rhs.entry_ == entry_)
        return *this;
    JobCounterEntry::release(entry_);
    entry_ = rhs.entry_;
    entry_->ref_.fetch_add(1, std::memory_order_relaxed);
    return *this;
}

Counter &Counter::operator==(Counter &&rhs)
{
    if (&rhs == this || rhs.entry_ == entry_)
        return *this;
    JobCounterEntry::release(entry_);
    entry_ = rhs.entry_;
    rhs.entry_ = nullptr;
    return *this;
}

Counter &Counter::operator+=(const Counter &rhs)
{
    entry_->cnt_.fetch_add(rhs.entry_->cnt_);
    return *this;
}

Counter JobBuilder::extract_wait_counter()
{
    assert(wait_counter_.is_valid());
    return std::move(wait_counter_);
}

JobBuilder::~JobBuilder() {}

void JobBuilder::dispatch(const std::string &name, JobBuilder::JobFunc func)
{
    assert(wait_counter_.is_valid());
    accumulate_counter_.add();
    // todo
}

void JobBuilder::dispatch_wait(const Counter &counter)
{
    wait_counter_ = counter;
}

void JobBuilder::dispatch_fence_explicitly()
{
    // todo
}

} // namespace cloud
