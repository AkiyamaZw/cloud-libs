// #include "graphics/vulkan/gpu_resource_manager.h"
// #include "graphics/vulkan/vulkan_interface.h"
// #include "graphics/vulkan/vulkan_device_context.h"
// #include "core/data_structure/memory.h"
// #include "core/runtime_log.h"

// namespace cloud::vulkan
// {
// GPUResourceManager::GPUResourceManager(VkDevice device,
// 									   VmaAllocator allocator,
// 									   VkDescriptorPool descriptor_pool,
// 									   uint32_t &current_frame,
// 									   ComponentResource &device_resource,
// 									   DefaultResources &default_resource,
// 									   std::vector<ResourceUpdate> &resource_deletion_queue,
// 									   std::vector<DescriptorSetUpdate> &descriptor_set_updates,
// 									   bool debug_utils_extension_present)
// 	: device_(device)
// 	, allocator_(allocator)
// 	, current_frame_(current_frame)
// 	, device_resource_(device_resource)
// 	, default_resource_(default_resource)
// 	, resource_deletion_queue_(resource_deletion_queue)
// 	, descriptor_set_updates_(descriptor_set_updates)
// 	, debug_utils_extension_present_(debug_utils_extension_present)
// {
// }

// void GPUResourceManager::SetResourceName(VkObjectType type, uint64_t handle, const char *name)
// {
// 	if (debug_utils_extension_present_)
// 	{
// 		infra::SetResourceName(device_, type, handle, name);
// 	}
// }

// SamplerHandle GPUResourceManager::CreateSampler(const SamplerCreation &creation)
// {
// 	SamplerHandle handle = {device_resource_.samplers.FetchResource()};
// 	if (handle.index == ResourcePool::INVALID_NUM)
// 	{
// 		return handle;
// 	}
// 	Sampler *sampler = Access<Sampler>(handle.index, device_resource_.samplers);
// 	infra::CreateSampler(device_, creation, sampler->sampler);
// 	SetResourceName(
// 		VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(sampler->sampler), creation.name);
// 	return handle;
// }
// void GPUResourceManager::DestroySampler(const SamplerHandle &handle, const uint32_t &frame_index)
// {
// 	if (handle.index < device_resource_.samplers.GetCapacity())
// 	{
// 		resource_deletion_queue_.push_back(
// 			{ResourceUpdateType::Sampler, handle.index, frame_index});
// 	}
// 	else
// 	{
// 		WARN("release sampler handle with error handle index %d", handle.index);
// 	}
// }

// void GPUResourceManager::ReleaseResourcesInDeletionQueue() const
// {

// 	for (uint32_t i = resource_deletion_queue_.size() - 1; i >= 0; i--)
// 	{
// 		ResourceUpdate &r = resource_deletion_queue_[i];
// 		if (r.current_frame == -1)
// 		{
// 			continue;
// 		}
// 		switch (r.type)
// 		{
// 		case ResourceUpdateType::Buffer:
// 			DestroyBufferInstance(r.handle);
// 			break;
// 		case ResourceUpdateType::Texture:
// 			break;
// 		case ResourceUpdateType::Pipeline:
// 			break;
// 		case ResourceUpdateType::Sampler:
// 			DestroySamplerInstance(r.handle);
// 			break;
// 		case ResourceUpdateType::DescriptorSetLayout:
// 			break;
// 		case ResourceUpdateType::DescriptorSet:
// 			break;
// 		case ResourceUpdateType::RenderPass:
// 			break;
// 		case ResourceUpdateType::Framebuffer:
// 			break;
// 		case ResourceUpdateType::ShaderState:
// 			break;
// 		case ResourceUpdateType::TextureView:
// 			break;
// 		case ResourceUpdateType::PagePool:
// 			break;
// 		case ResourceUpdateType::Count:
// 			break;
// 		}
// 		r.current_frame = InvalidFrameID;
// 		std::swap(resource_deletion_queue_.back(), r);
// 		resource_deletion_queue_.pop_back();
// 	}
// }

// void GPUResourceManager::UpdateDynamicBuffer()
// {
// 	const uint32_t used_size = device_resource_.dynamic_allocated_size -
// 							   (device_resource_.dynamic_per_frame_size * previous_frame);
// 	device_resource_.dynamic_per_frame_size =
// 		std::max(used_size, device_resource_.dynamic_max_per_frame_size);
// 	device_resource_.dynamic_allocated_size =
// 		device_resource_.dynamic_per_frame_size * current_frame_;
// }

// void GPUResourceManager::UpdateDescriptorSet()
// {
// 	if (descriptor_set_updates_.size() > 0)
// 	{
// 		for (uint32_t i = descriptor_set_updates_.size() - 1; i >= 0; i--)
// 		{
// 			DescriptorSetUpdate &update = descriptor_set_updates_[i];
// 			UpdateDescriptorSetInternal(update);
// 			update.current_frame = InvalidFrameID;
// 			std::swap(descriptor_set_updates_.back(), update);
// 			descriptor_set_updates_.pop_back();
// 		}
// 	}
// }

// void GPUResourceManager::DestroySamplerInstance(ResourceHandle handle) const
// {
// 	if (auto sampler = static_cast<Sampler *>(device_resource_.samplers.Access(handle)))
// 	{
// 		vkDestroySampler(device_, sampler->sampler, nullptr);
// 	}
// 	device_resource_.samplers.ReleaseResource(handle);
// }

// BufferHandle GPUResourceManager::CreateBuffer(const BufferCreation &creation)
// {
// 	BufferHandle handle = {device_resource_.buffers.FetchResource()};
// 	if (handle.index == ResourcePool::INVALID_NUM)
// 	{
// 		return handle;
// 	}
// 	Buffer *buffer = Access<Buffer>(handle.index, device_resource_.buffers);
// 	buffer->name = creation.name;
// 	buffer->size = creation.size;
// 	buffer->usage_type = creation.usage_type;
// 	buffer->usage_flags = creation.usage_flags;
// 	buffer->handle = handle;
// 	buffer->global_offset = 0;
// 	buffer->parent_handle = BufferHandle{ResourcePool::INVALID_NUM};
// 	static const VkBufferUsageFlags buffer_usage_mask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
// 														VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
// 														VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
// 	const bool use_global_buffer = (creation.usage_flags & buffer_usage_mask) != 0;
// 	if (creation.usage_type == ResourceUsageType::Dynamic && use_global_buffer)
// 	{
// 		buffer->parent_handle = device_resource_.dynamic_buffer;
// 		return handle;
// 	};
// 	VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
// 	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | creation.usage_flags;
// 	buffer_create_info.size = creation.size > 0 ? creation.size : 1;

// 	VmaAllocationCreateInfo allocation_create_info{};
// 	allocation_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
// 	allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
// 	VmaAllocationInfo allocation_info{};
// 	auto succ = vmaCreateBuffer(allocator_,
// 								&buffer_create_info,
// 								&allocation_create_info,
// 								&buffer->buffer,
// 								&buffer->allocation,
// 								&allocation_info);
// 	check_vk(succ);
// 	SetResourceName(VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer->buffer, creation.name);
// 	buffer->memory = allocation_info.deviceMemory;
// 	if (creation.initial_data)
// 	{
// 		void *data;
// 		vmaMapMemory(allocator_, buffer->allocation, &data);
// 		memcpy(data, creation.initial_data, (size_t)creation.size);
// 		vmaUnmapMemory(allocator_, buffer->allocation);
// 	}
// 	return handle;
// }

// void GPUResourceManager::DestroyBuffer(const BufferHandle &handle, const uint32_t &frame_index)
// {
// 	if (handle.index < device_resource_.buffers.GetCapacity())
// 	{
// 		resource_deletion_queue_.push_back({ResourceUpdateType::Buffer, handle.index, frame_index});
// 	}
// 	else
// 	{
// 		WARN("Graphics error: try to free invalid buffer %d", handle.index);
// 	}
// }

// void GPUResourceManager::DestroyBufferInstance(ResourceHandle handle) const
// {
// 	auto *buffer = static_cast<Buffer *>(device_resource_.buffers.Access(handle));
// 	if (buffer && buffer->parent_handle.index == ResourcePool::INVALID_NUM)
// 	{
// 		vmaDestroyBuffer(allocator_, buffer->buffer, buffer->allocation);
// 	}
// 	device_resource_.buffers.ReleaseResource(handle);
// }

// void GPUResourceManager::DestroyDescriptorSet(const DescriptorSetHandle &handle,
// 											  const uint32_t &frame_index)
// {
// 	if (handle.index < device_resource_.descriptor_sets.GetCapacity())
// 	{
// 		resource_deletion_queue_.push_back(
// 			{ResourceUpdateType::DescriptorSet, handle.index, frame_index});
// 	}
// 	else
// 	{
// 		WARN("Graphics error: try to free invalid descriptorset %d", handle.index);
// 	}
// }

// void *GPUResourceManager::DynamicAllocate(uint32_t size)
// {
// 	void *memory = device_resource_.dynamic_mapped_memory + device_resource_.dynamic_allocated_size;
// 	device_resource_.dynamic_allocated_size += (uint32_t)cloud::MemoryAlign(size, GUboAlignment);
// 	return memory;
// }

// void *GPUResourceManager::MapBuffer(const render::MapBufferParameter &param)
// {
// 	if (param.handle.index == ResourcePool::INVALID_NUM)
// 		return nullptr;
// 	Buffer *buffer = static_cast<Buffer *>(device_resource_.buffers.Access(param.handle.index));
// 	if (buffer->parent_handle.index == device_resource_.dynamic_buffer.index)
// 	{
// 		buffer->global_offset = device_resource_.dynamic_allocated_size;
// 		return DynamicAllocate(param.size == 0 ? buffer->size : param.size);
// 	}
// 	void *data;
// 	vmaMapMemory(allocator_, buffer->allocation, &data);
// 	return data;
// }

// void GPUResourceManager::UnMapBuffer(const render::MapBufferParameter &param)
// {
// 	if (param.handle.index == ResourcePool::INVALID_NUM)
// 		return;
// 	Buffer *buffer = static_cast<Buffer *>(device_resource_.buffers.Access(param.handle.index));
// 	if (buffer->parent_handle.index == device_resource_.dynamic_buffer.index)
// 		return;
// 	vmaUnmapMemory(allocator_, buffer->allocation);
// }

// void GPUResourceManager::UpdateDescriptorSetInternal(DescriptorSetUpdate &update)
// {
// 	DescriptorSetHandle dummy_delete_dsh = {device_resource_.descriptor_sets.FetchResource()};
// 	DescriptorSet *dummy_delete_ds =
// 		Access<DescriptorSet>(dummy_delete_dsh.index, device_resource_.descriptor_sets);
// 	DescriptorSet *descriptor_set =
// 		Access<DescriptorSet>(update.descriptor_set.index, device_resource_.descriptor_sets);
// 	const DescriptorSetLayout *descriptor_set_layout = descriptor_set->layout;
// 	dummy_delete_ds->descriptor_set = descriptor_set->descriptor_set;
// 	dummy_delete_ds->bindings = nullptr;
// 	dummy_delete_ds->resources = nullptr;
// 	dummy_delete_ds->samplers = nullptr;
// 	dummy_delete_ds->num_resources = 0;
// 	DestroyDescriptorSet(dummy_delete_dsh, current_frame_);

// 	VkWriteDescriptorSet descriptor_write[8];
// 	VkDescriptorBufferInfo buffer_info[8];
// 	VkDescriptorImageInfo image_info[8];
// 	Sampler *vk_default_sampler =
// 		Access<Sampler>(default_resource_.default_sampler.index, device_resource_.samplers);

// 	VkDescriptorSetAllocateInfo alloc_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
// 	alloc_info.descriptorPool = device_resource_.descriptor_pool;
// }

// } // namespace cloud::vulkan
