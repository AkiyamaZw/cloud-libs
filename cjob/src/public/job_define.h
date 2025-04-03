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

namespace cloud::js::internal
{
template <typename T>
class ResourcePool;
}

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
    Latent = 0,
    RenderPath = 1,
    CriticalPath = 2,
    Immediate = 3,
    Count
};

/**
 * @brief the args of job needed while join to the job system.
 *
 */
struct JobArgs
{
};

class JobCounterEntry;
class JobWaitEntry;
using CounterPool = internal::ResourcePool<JobCounterEntry>;
using JobWaitListEntryPool = internal::ResourcePool<JobWaitEntry>;

static constexpr size_t MAX_JOB_COUNT = 16384;
static_assert(MAX_JOB_COUNT <= 0x7FFE, "MAX_JOB_COUNT is 16384");
using JobQueue = StealableQueue<uint16_t, MAX_JOB_COUNT>;

} // namespace cloud::js