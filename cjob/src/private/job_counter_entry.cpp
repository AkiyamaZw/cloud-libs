#include "job_counter_entry.h"

namespace cloud::js
{
JobCounterEntry::JobCounterEntry()
    : internal::CountablePoolableObject<JobCounterEntry>()
{
}

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

void JobCounterEntry::init() {}

void JobCounterEntry::reset()
{
    finish_submit_job_ = false;
    wait_job_list_.clear();
    wait_counter_.set_cnt(0);
    wait_counter_list_.clear();
    CountablePoolableObject<JobCounterEntry>::reset();
}

bool JobCounterEntry::ready_to_release() const
{
    return wait_counter_.get_cnt() == 0 && get_ref() == 0;
}

void JobCounterEntry::on_counter_signal() {}

void JobCounterEntry::on_counter_destroyed() {}

void JobCounterEntry::accumulate() { wait_counter_.add_cnt(1); }

void JobCounterEntry::signal() { wait_counter_.sub_cnt(1); }

uint32_t JobCounterEntry::get_waits() const { return wait_counter_.get_cnt(); }

} // namespace cloud::js