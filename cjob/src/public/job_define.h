#pragma once
#include <cstdint>

namespace cloud
{
enum class JobPriority
{
    High,
    Low,
    Streaming,
    Count
};

struct JobContext
{
    volatile long counter{0};
    JobPriority priority{JobPriority::High};
};

struct JobArgs
{
    uint32_t job_index;
    uint32_t group_id;
    uint32_t group_index;
    bool is_first_job_in_group;
    bool is_last_job_in_group;
    void *shared_memory;
};
} // namespace cloud