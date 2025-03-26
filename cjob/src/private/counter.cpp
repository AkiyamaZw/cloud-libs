#include "counter.h"
#include <cassert>
#include <fstream>
#include <format>
#include <set>
#include "job_system.h"

namespace cloud
{
JobCounterEntry::JobCounterEntry() {}

JobCounterEntry::~JobCounterEntry()
{
    wait_list_head_.clear();
    next_runable_jobs_ = 0;
}

void JobCounterEntry::add_dep_jobs(JobWaitListEntry *entry)
{
    std::lock_guard lock(dep_jobs_lock_);
    wait_list_head_.push_back(entry);
}

void JobCounterEntry::add_dep_counters(JobCounterEntry *counter)
{
    std::lock_guard lock(dep_counter_lock_);
    wait_counter_list_.push_back(counter);
}

void JobCounterEntry::init()
{
    assert(finish_submit_.load() == State::Released);
    finish_submit_.store(State::Setupped);
}

void JobCounterEntry::reset()
{
    assert(finish_submit_.load() != State::Released);
    wait_list_head_.clear();
    next_runable_jobs_ = 0;
    finish_submit_.store(State::Released);
    set_cnt(0);
    wait_counter_list_.clear();
}

bool JobCounterEntry::ready_to_release() const
{
    return finish_submit_.load() == State::FinishAddDepend && get_cnt() == 0;
}

Counter::Counter(JobCounterEntry *entry) { entry_ = entry; }

Counter::~Counter() {}

Counter::Counter(const Counter &rhs) { entry_ = rhs.entry_; }

Counter::Counter(Counter &&rhs) noexcept
{
    entry_ = rhs.entry_;
    rhs.entry_ = nullptr;
}

Counter &Counter::operator=(const Counter &rhs)
{
    if (&rhs == this || rhs.entry_ == entry_)
        return *this;

    entry_ = rhs.entry_;
    return *this;
}

Counter &Counter::operator=(Counter &&rhs)
{
    if (&rhs == this || rhs.entry_ == entry_)
        return *this;
    entry_ = rhs.entry_;
    rhs.entry_ = nullptr;
    return *this;
}

Counter &Counter::operator+=(const Counter &rhs)
{
    // this means wait rhs to finish
    entry_->add_cnt(1);
    rhs.entry_->add_dep_counters(entry_);
    return *this;
}

bool Counter::operator==(const Counter &rhs) const
{
    return get_id() == rhs.get_id();
}

void Counter::finish_submit_job()
{
    if (is_valid())
    {
        entry_->finish_submit_.store(JobCounterEntry::State::FinishAddDepend);
    }
}

Counter Counter::create(JobSystem &js)
{
    return Counter(js.create_entry_counter());
}

// void RunContext::export_grapviz(const std::string &path)
// {
// std::ofstream out(path, std::ios::out | std::ios::binary);
// if (out.fail())
// {
//     return;
// }
// out << "digraph framegraph {\n";
// out << "rankdir = TB\n";
// out << "bgcolor = white\n";
// out << "node [shape=rectangle, fontname=\"helvetica\", fontsize=10]\n\n";

// const std::string line_color = "green3";
// const std::string unvalid_line_color = "crimson";
// std::set<const Counter *> counter_sets;
// std::set<const Job *> job_sets;

// for (const auto &[id, counter] : wait_counter_queue_)
// {
//     counter_sets.insert(&counter);

//     auto job_list_ptr = counter.get_jobs();
//     while (job_list_ptr != nullptr)
//     {
//         job_sets.insert(job_list_ptr->job);
//         counter_sets.insert(job_list_ptr->accumulate_counter);
//         job_list_ptr = job_list_ptr->next;
//     }
// }
// for (const auto *counter : counter_sets)
// {
//     out << "\"NCOUNTER" << counter->get_id() << "\" "
//         << counter_grapviz(*counter) << "\n";
// }

// for (const auto *job : job_sets)
// {
//     out << "\"NJOB" << job->get_index() << "\" " << job_graphviz(*job)
//         << "\n";
// }

// for (const auto &[id, counter] : wait_counter_queue_)
// {
//     out << "NCOUNTER" << id << "-> {";

//     auto job_list_ptr = counter.get_jobs();
//     while (job_list_ptr != nullptr)
//     {
//         auto job = job_list_ptr->job;
//         out << "NJOB" << job->get_index() << " ";
//         job_list_ptr = job_list_ptr->next;
//     }
//     out << "} [color=" << line_color << "]\n";
// }
// for (const auto &[id, counter] : wait_counter_queue_)
// {
//     auto job_list_ptr = counter.get_jobs();
//     while (job_list_ptr != nullptr)
//     {
//         out << "NJOB" << job_list_ptr->job->get_index() << "-> {";
//         out << "NCOUNTER" << job_list_ptr->accumulate_counter.get_id()
//             << " ";
//         out << "} [color=" << unvalid_line_color << "]\n";
//         job_list_ptr = job_list_ptr->next;
//     }
// }
// out << "}" << std::endl;
// }

// std::string RunContext::counter_grapviz(const Counter &counter)
// {
//     std::string s = std::format("[label=\"counter:{}\\n pre_jobs: {}\", "
//                                 "style=filled, fillcolor={}]",
//                                 counter.get_id(),
//                                 counter.get_cnt(),
//                                 "cyan");
//     s.shrink_to_fit();
//     return s;
// }

// std::string RunContext::job_graphviz(const Job &job) const
// {
//     std::string s =
//         std::format("[label=\"job:{}\\n id: {}\", style=filled,
//         fillcolor={}]",
//                     job.get_name(),
//                     job.get_index(),
//                     "red");
//     s.shrink_to_fit();
//     return s;
// }

} // namespace cloud
