#include "job.h"
namespace cloud
{

Job::~Job() {}

Job::Job(const Job &rhs)
{
    task = rhs.task;
    running_job_count_.store(rhs.running_job_count_.load());
}

Job &Job::operator=(const Job &rhs)
{
    task = rhs.task;
    running_job_count_.store(rhs.running_job_count_.load());
    return *this;
}

void Job::reset()
{
    running_job_count_.store(0);
    handle_.index = JobHandle::INVALID_HANDLE_INDEX;
}

JobPool::~JobPool()
{
    free_list_.clear();

    for (auto ptr : job_pool_)
    {
        delete ptr;
        ptr = nullptr;
    }
    job_pool_.clear();
}

Job *JobPool::get()
{

    Job *job = nullptr;
    if (free_list_.empty())
    {
        job = create();
    }
    else
    {
        std::lock_guard lock(mutex_);
        uint32_t index = free_list_.front();
        free_list_.pop_front();
        job = job_pool_[index];
    }
    assert(job != nullptr);
    return job;
}

Job *JobPool::at(uint32_t index)
{
    std::lock_guard lock(mutex_);
    return job_pool_[index];
}

Job *JobPool::at(const JobHandle &handle)
{
    if (!handle.is_valid())
        return nullptr;
    else
    {
        return at(handle.index);
    }
}

void JobPool::release(Job *job)
{
    assert(job != nullptr);
    assert(job->handle_.is_valid());
    job->reset();
    std::lock_guard lock(mutex_);
    free_list_.push_back(job->handle_.index);
}

Job *JobPool::create()
{
    std::lock_guard lock(mutex_);
    Job *job = new Job();
    job->handle_ = JobHandle(job_pool_.size());
    job_pool_.push_back(job);
    return job;
}

} // namespace cloud
