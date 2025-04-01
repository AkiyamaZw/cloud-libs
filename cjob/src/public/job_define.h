#pragma once
#include <cstdint>
#include <atomic>
#include <functional>
#include <cassert>
#include <deque>
#include <mutex>
#include <vector>
#include <memory>
#include "stealable_queue.h"
#include <format>

namespace cloud::js
{
struct JobArgs;
using JobFunc = std::function<void(JobArgs &)>;

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
    uint32_t group_index;
    bool is_first_job_in_group;
    bool is_last_job_in_group;
    void *shared_memory;
};

static constexpr size_t MAX_JOB_COUNT = 16384;
static_assert(MAX_JOB_COUNT <= 0x7FFE, "MAX_JOB_COUNT is 16384");
using JobQueue = StealableQueue<uint16_t, MAX_JOB_COUNT>;

} // namespace cloud::js