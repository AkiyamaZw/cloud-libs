#pragma once
#include <functional>

namespace cloud
{
enum class JobState
{
    Setup,
    Started,
    Pending,
    Processing,
    Syspended
};

struct Job
{
    std::function<void()> task;
    JobState state_;
};

} // namespace cloud