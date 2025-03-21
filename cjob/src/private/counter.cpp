#include "counter.h"
#include <cassert>
#include <fstream>
#include <format>
#include <set>

namespace cloud
{

CJob::CJob(const CJob &rhs)
{
    job = rhs.job;
    job->ref.fetch_add(1);
}

JobCounterEntry::~JobCounterEntry()
{
    auto *ptr = wait_list_head_;
    while (ptr != nullptr)
    {
        auto *next = ptr->next;
        delete ptr;
        ptr = next;
    }
}

void JobCounterEntry::add_dep_jobs(CJob &job, Counter &counter)
{
    std::lock_guard lock(dep_jobs_lock_);
    // dep_jobs_.push_back(job);
    auto new_entry = new JobWaitListEntry();
    new_entry->cjob = job;
    new_entry->accumulate_counter = counter;
    new_entry->next = wait_list_head_;
    wait_list_head_ = new_entry;
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

Counter &Counter::operator=(Counter &&rhs)
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

    if (entry_->wait_list_head_ == nullptr)
        entry_->wait_list_head_ = rhs.entry_->wait_list_head_;
    else
    {
        auto next_ptr = entry_->wait_list_head_;
        while (next_ptr->next != nullptr)
        {
            next_ptr = next_ptr->next;
        }
        next_ptr->next = rhs.entry_->wait_list_head_;
    }
    return *this;
}

bool Counter::operator==(const Counter &rhs) const
{
    return get_id() == rhs.get_id();
}

void Counter::add_dep_task(CJob &job, Counter &accu_counter) const
{
    entry_->add_dep_jobs(job, accu_counter);
}

JobBuilder::JobBuilder() {}

JobBuilder::~JobBuilder() {}

Counter JobBuilder::extract_wait_counter()
{
    assert(accumulate_counter_.is_valid());

    return accumulate_counter_;
}

void JobBuilder::dispatch(const std::string &name, JobBuilder::JobFunc func)
{
    assert(wait_counter_.is_valid());
    accumulate_counter_.add();
    CJob job(name, func);
    wait_counter_.add_dep_task(job, accumulate_counter_);
    RunContext::get_context()->push_wait_jobs(wait_counter_);
}

void JobBuilder::dispatch_fence_explicitly()
{
    RunContext *context = RunContext::get_context();
    wait_counter_ = accumulate_counter_;
    accumulate_counter_ = Counter();
    accumulate_counter_.set_cnt(wait_counter_.get_cnt() + 1);
}

void JobBuilder::dispatch_wait(const Counter &counter)
{
    assert(wait_counter_.get_cnt() == 0);
    wait_counter_ = counter;
    accumulate_counter_.set_cnt(wait_counter_.get_cnt());
}

void RunContext::push_wait_jobs(Counter &counter)
{
    std::lock_guard lock(cur_counters_map_mutex_);

    auto cnt_iter = wait_counter_queue_.find(counter.get_id());
    if (cnt_iter == wait_counter_queue_.end())
    {
        wait_counter_queue_[counter.get_id()] = counter;
    }

    auto iter = cur_counters_.find(std::this_thread::get_id());
    if (iter == cur_counters_.end())
    {
        cur_counters_[std::this_thread::get_id()] = counter;
    }
    else
    {
        if (iter->second != counter)
        {
            cur_counters_[std::this_thread::get_id()] = counter;
        }
    }
}

void RunContext::export_grapviz(const std::string &path)
{
    std::ofstream out(path, std::ios::out | std::ios::binary);
    if (out.fail())
    {
        return;
    }
    out << "digraph framegraph {\n";
    out << "rankdir = TB\n";
    out << "bgcolor = white\n";
    out << "node [shape=rectangle, fontname=\"helvetica\", fontsize=10]\n\n";

    const std::string line_color = "green3";
    const std::string unvalid_line_color = "crimson";
    std::set<const Counter *> counter_sets;
    std::set<const CJob *> job_sets;

    for (const auto &[id, counter] : wait_counter_queue_)
    {
        counter_sets.insert(&counter);

        auto job_list_ptr = counter.get_jobs();
        while (job_list_ptr != nullptr)
        {
            job_sets.insert(&job_list_ptr->cjob);
            counter_sets.insert(&job_list_ptr->accumulate_counter);
            job_list_ptr = job_list_ptr->next;
        }
    }
    for (const auto *counter : counter_sets)
    {
        out << "\"NCOUNTER" << counter->get_id() << "\" "
            << counter_grapviz(*counter) << "\n";
    }

    for (const auto *job : job_sets)
    {
        out << "\"NJOB" << job->get_id() << "\" " << job_graphviz(*job) << "\n";
    }

    for (const auto &[id, counter] : wait_counter_queue_)
    {
        auto id = counter.get_id();
        out << "NCOUNTER" << id << "-> {";

        auto job_list_ptr = counter.get_jobs();
        while (job_list_ptr != nullptr)
        {
            auto job = job_list_ptr->cjob;
            out << "NJOB" << job.get_id() << " ";
            job_list_ptr = job_list_ptr->next;
        }
        out << "} [color=" << line_color << "]\n";
    }
    for (const auto &[id, counter] : wait_counter_queue_)
    {
        auto job_list_ptr = counter.get_jobs();
        while (job_list_ptr != nullptr)
        {
            out << "NJOB" << job_list_ptr->cjob.get_id() << "-> {";
            out << "NCOUNTER" << job_list_ptr->accumulate_counter.get_id()
                << " ";
            out << "} [color=" << unvalid_line_color << "]\n";
            job_list_ptr = job_list_ptr->next;
        }
    }
    out << "}" << std::endl;
}

std::string RunContext::counter_grapviz(const Counter &counter)
{
    std::string s = std::format("[label=\"counter:{}\\n pre_jobs: {}\", "
                                "style=filled, fillcolor={}]",
                                counter.get_id(),
                                counter.get_cnt(),
                                "cyan");
    s.shrink_to_fit();
    return s;
}

std::string RunContext::job_graphviz(const CJob &job) const
{
    std::string s =
        std::format("[label=\"job:{}\\n id: {}\", style=filled, fillcolor={}]",
                    job.get_name(),
                    job.get_id(),
                    "red");
    s.shrink_to_fit();
    return s;
}

} // namespace cloud
