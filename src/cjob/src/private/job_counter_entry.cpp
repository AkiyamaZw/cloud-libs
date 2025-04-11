#include "job_counter_entry.h"

namespace cloud::js
{

SubToTriggerCountable::SubToTriggerCountable(uint32_t level)
    : Countable()
    , level_(level)
{
}
SubToTriggerCountable::~SubToTriggerCountable() {}

void SubToTriggerCountable::register_callback(const std::string &name,
                                              Callback callback)
{
    std::lock_guard lock(map_mutex_);
    callbacks_.emplace(name, callback);
}

void SubToTriggerCountable::unregister_callback(const std::string &name)
{

    std::lock_guard lock(map_mutex_);
    auto iter = callbacks_.find(name);
    if (iter == callbacks_.end())
        return;
    callbacks_.erase(iter);
}

uint16_t SubToTriggerCountable::sub_cnt(uint16_t i, JobCounterEntry *entry)
{
    uint16_t latest = Countable::sub_cnt(i);
    if (entry != nullptr && get_cnt() == level_)
    {
        for (auto &[_, cb] : callbacks_)
        {
            cb(entry);
        }
    }
    return latest;
}

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

void JobCounterEntry::init(JobCounterEntry::Callback callback)
{
    wait_counter_.register_callback("", callback);
}

void JobCounterEntry::reset()
{
    finish_submit_job_ = false;
    {
        std::lock_guard lock(dep_jobs_lock_);
        wait_job_list_.clear();
    }
    {
        std::lock_guard lock(dep_counter_lock_);
        wait_counter_list_.clear();
    }
    wait_counter_.reset();
    CountablePoolableObject<JobCounterEntry>::reset();
}

bool JobCounterEntry::ready_to_release() const
{
    return wait_counter_.get_cnt() == 0 && get_ref() == 0;
}

void JobCounterEntry::on_counter_signal() {}

void JobCounterEntry::on_counter_destroyed() {}

void JobCounterEntry::accumulate() { wait_counter_.add_cnt(1); }

void JobCounterEntry::signal() { wait_counter_.sub_cnt(1, this); }

uint32_t JobCounterEntry::get_waits() const { return wait_counter_.get_cnt(); }

} // namespace cloud::js