#include "graphics/vulkan/GPUTimeStampManager.h"

namespace cloud::vulkan
{

GPUTimestampManager::GPUTimestampManager(uint16_t queries_per_frame, uint16_t max_frames)
    :queries_per_frame_(queries_per_frame)
{
    const uint32_t data_per_query = 2; // [start, end]
    const size_t free_space = sizeof(GPUTimestamp) * queries_per_frame * max_frames +
        sizeof(uint64_t) * queries_per_frame * max_frames * data_per_query;
    uint8_t *raw_memory = malloc(free_space);
    timestamps_ = reinterpret_cast<GPUTimestamp *>(raw_memory);
    timestamps_data_ = reinterpret_cast<uint64_t *>(raw_memory + sizeof(GPUTimestamp) * queries_per_frame * max_frames);
    Reset();
}

GPUTimestampManager::~GPUTimestampManager()
{
    free(timestamps_data_);
    timestamps_data_ = nullptr;
}

bool GPUTimestampManager::HasValidQueries() const
{
    return current_query_ > 0 && depth_== 0;
}

void GPUTimestampManager::Reset()
{
    current_query_ = 0;
    parent_index_ = 0;
    current_frame_resolved_ = false;
    depth_ = 0;
}

uint32_t GPUTimestampManager::Resolve(uint32_t current_frame, GPUTimestamp *timestamps_to_fill)
{
    memcpy(timestamps_to_fill, &timestamps_[current_frame * queries_per_frame_], sizeof(GPUTimestamp) * current_query_);
    return current_query_;
}

uint32_t GPUTimestampManager::Push(uint32_t current_frame, const char *name)
{
    uint32_t query_index = (current_frame * queries_per_frame_) + current_query_;
    GPUTimestamp& timestamp = timestamps_[query_index];
    timestamp.parent_index = (uint64_t)parent_index_;
    timestamp.start = query_index * 2;
    timestamp.end = timestamp.start + 1;
    timestamp.name = name;
    timestamp.depth = depth_++;

    parent_index_ = current_query_;
    ++current_query_;
    return (query_index * 2);
}

uint32_t GPUTimestampManager::Pop(uint32_t current_frame)
{
    uint32_t query_index = (current_frame * queries_per_frame_) + parent_index_;
    GPUTimestamp& timestamp = timestamps_[query_index];
    parent_index_ = timestamp.parent_index;
    --depth_;
    return (query_index * 2) + 1;
}
}