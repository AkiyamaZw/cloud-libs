#include "job_counter_entry.h"

namespace cloud
{
JobCounterEntry::JobCounterEntry() {}

JobCounterEntry::~JobCounterEntry()
{
    assert(wait_job_list_.empty());
    wait_job_list_.clear();
    assert(wait_counter_list_.empty());
    wait_counter_list_.clear();
}

void JobCounterEntry::add_dep_jobs(JobWaitEntry *entry)
{
    std::lock_guard lock(dep_jobs_lock_);
    wait_job_list_.push_back(entry);
}

void JobCounterEntry::add_dep_counters(JobCounterEntry *counter)
{
    std::lock_guard lock(dep_counter_lock_);
    wait_counter_list_.push_back(counter);
}

void JobCounterEntry::init()
{
    assert(state_.load() == State::Released);
    state_.store(State::Setupped);
}

void JobCounterEntry::reset()
{
    assert(state_.load() != State::Released);
    wait_job_list_.clear();
    state_.store(State::Released);
    set_cnt(0);
    wait_counter_list_.clear();
}

bool JobCounterEntry::ready_to_release() const
{
    return state_.load() == State::FinishAddDepend && get_cnt() == 0;
}

void JobCounterEntry::on_counter_signal() {}

void JobCounterEntry::on_counter_destroyed() {}
} // namespace cloud