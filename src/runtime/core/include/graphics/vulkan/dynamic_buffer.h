#pragma once

#include <cstdint>
#include <cassert>
#include "graphics/core/gpu_resource.h"
#include "core/data_structure/memory.h"

namespace cloud::vulkan
{

// 环形 bump 分配器：一个大 buffer 切 N 槽，每帧轮换写入
// 流程: AdvanceSlot() 切换到当前槽起始 → Allocate() 线性分配 →
//       提交后该槽 fence 被触发 → 下次复用同一槽时等 fence 保证 GPU 已读完
struct DynamicBuffer
{
	uint32_t slot_count{0};                // 槽数量
	uint32_t per_frame_size{0};            // 每槽大小字节
	uint32_t alignment{0};                 // 分配对齐
	uint32_t max_per_frame_size{0};        // 单帧最大用量 (诊断/调参用)
	ResourceHandle buffer;                 // 底层 VkBuffer handle
	uint8_t *mapped_memory{nullptr};       // 持久映射
	uint32_t allocated_size{0};            // 当前分配偏移 (绝对位置)

	struct MapBufferParameters
	{
		ResourceHandle handle;
		uint32_t offset{0};
		uint32_t size{0};
	};

	// 显式设置行为参数（必须在 StartFrame 使用之前调用）
	void Init(uint32_t slot_count, uint32_t per_frame_size, uint32_t alignment)
	{
		this->slot_count = slot_count;
		this->per_frame_size = per_frame_size;
		this->alignment = alignment;
	}

	// 在当前槽内 bump 分配，返回映射指针，超限返回 nullptr
	void *Allocate(uint32_t size)
	{
		const uint32_t aligned = (uint32_t)cloud::MemoryAlign(size, alignment);
		const uint32_t total = per_frame_size * slot_count;
		const uint32_t new_size = allocated_size + aligned;
		assert(new_size <= total && "Dynamic buffer overflow");
		if (new_size > total)
			return nullptr;
		void *memory = mapped_memory + allocated_size;
		allocated_size = new_size;
		return memory;
	}

	// 切换到下一帧槽位，记录上一帧实际用量
	void AdvanceSlot(uint32_t prev_slot, uint32_t curr_slot)
	{
		const uint32_t used = allocated_size - (per_frame_size * prev_slot);
		if (used > max_per_frame_size)
			max_per_frame_size = used;
		allocated_size = per_frame_size * curr_slot;
	}
};

} // namespace cloud::vulkan
