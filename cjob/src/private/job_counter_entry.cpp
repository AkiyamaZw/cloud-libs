#include "job_counter_entry.h"

namespace cloud::js
{
JobCounterEntry::JobCounterEntry()
{
    ref_counter_.set_signal_callback(0, [this](Countable &cc) {
        if (ready_to_release())
        {
            on_counter_destroyed();
            pool_->release(this);
        }
    });
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

void JobCounterEntry::init()
{
    set_state(State::Setupped);
    ref_counter_.set_cnt(0);
}

void JobCounterEntry::reset()
{
    set_state(State::Released);
    wait_job_list_.clear();
    wait_counter_.set_cnt(0);
    wait_counter_list_.clear();
    ref_counter_.set_cnt(0);
}

bool JobCounterEntry::ready_to_release() const
{
    return wait_counter_.get_cnt() == 0 && ref_counter_.get_cnt() == 0;
}

void JobCounterEntry::on_counter_signal() {}

void JobCounterEntry::on_counter_destroyed() {}
void JobCounterEntry::set_state(State state)
{
    if (state_.load() == State::Released && state == State::FinishAddDepend)
    {
        assert(false);
    }
    if (state_.load() != State::Released && state == State::Setupped)
    {
        assert(false);
    }
    if (state_.load() == State::Released && state == State::Released)
    {
        assert(false);
    }
    state_.store(state);
}

void JobCounterEntry::accumulate() { wait_counter_.add_cnt(1); }

void JobCounterEntry::signal() { wait_counter_.sub_cnt(1); }

uint32_t JobCounterEntry::get_waits() const { return wait_counter_.get_cnt(); }

} // namespace cloud::js