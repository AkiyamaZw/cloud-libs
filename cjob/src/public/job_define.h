#pragma once
#include <cstdint>
#include <atomic>

namespace cloud
{
/**
 * @brief define the priority of job
 *
 */
enum class JobPriority
{
    High,
    Low,
    Streaming,
    Count
};

/**
 * @brief job execute context
 *
 */
struct JobContext
{
    std::atomic_long counter{0};
    JobPriority priority{JobPriority::High};
};

/**
 * @brief the args of job needed while join to the job system.
 *
 */
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