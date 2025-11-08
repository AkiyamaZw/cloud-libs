#pragma once
#include <cstdint>

#include "spdlog/fmt/bundled/chrono.h"

namespace cloud::vulkan
{
struct GPUTimestamp
{
    uint32_t start;
    uint32_t end;
    double elapsed_ms;
    uint16_t parent_index;
    uint16_t depth;
    uint32_t color;
    uint32_t frame_index;
    const char* name;
};

/*
 * for n frame swapchain image cache timestamp
 * | frame1        | frame 2 |... | frame k|
 * | n queries data| ...     | ...| ...    |
 * n = queries_per_frame
 * k = max_frames
 */
class GPUTimestampManager final
{
public:
    GPUTimestampManager(uint16_t queries_per_frame, uint16_t max_frames);
    ~GPUTimestampManager();

    bool HasValidQueries() const;
    void Reset();
    uint32_t Resolve(uint32_t current_frame, GPUTimestamp* timestamps_to_fill);
    uint32_t Push(uint32_t current_frame, const char* name);
    uint32_t Pop(uint32_t current_frame);

private:
    /* array of GPUTimestamp with size queries_per_frame * max_frames */
    GPUTimestamp* timestamps_{nullptr};
    /* array of uint64_t with size 2 * queries_per_frame * max_frames */
    uint64_t* timestamps_data_{nullptr};

    uint32_t queries_per_frame_{0};

    uint32_t current_query_{0};
    uint32_t parent_index_{0};
    uint32_t depth_{0};
    bool current_frame_resolved_{false};
};
}


