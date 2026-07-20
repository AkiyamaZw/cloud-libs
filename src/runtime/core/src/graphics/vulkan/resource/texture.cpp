#include "graphics/vulkan/resource/texture.h"
#include "core/data_structure/resource_pool.h"
#include "graphics/vulkan/device_data.h"
#include "graphics/vulkan/vulkan_interface.h"

namespace cloud::vulkan
{
void CreateVkTextureInner(const DeviceData &device_data,
						  const TextureCreation &creation,
						  const ResourceData &resource_data,
						  const ResourceHandle &handle,
						  Texture &texture)
{
	COPY_MEMBER(texture, creation, width);
	COPY_MEMBER(texture, creation, height);
	COPY_MEMBER(texture, creation, depth);
	COPY_MEMBER(texture, creation, mipmaps);
	COPY_MEMBER(texture, creation, flags);
	COPY_MEMBER(texture, creation, type);
	COPY_MEMBER(texture, creation, format);

	texture.sampler = nullptr;
	VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_create_info.format = texture.format;
	image_create_info.flags = 0;
	ToVKEnum(texture.type, image_create_info.imageType);
	COPY_MEMBER(image_create_info.extent, texture, width);
	COPY_MEMBER(image_create_info.extent, texture, height);
	COPY_MEMBER(image_create_info.extent, texture, depth);
	image_create_info.mipLevels = texture.mipmaps;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	const bool is_rt = HasAny(creation.flags, TextureFlags::Mask::RenderTarget);
	const bool is_compute = HasAny(creation.flags, TextureFlags::Mask::Compute);
	image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	image_create_info.usage |= is_compute ? VK_IMAGE_USAGE_STORAGE_BIT : 0;
	if (utility::HasDepthOrStencil(creation.format))
	{
		image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_create_info.usage |= is_rt ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
	}
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo mem_info{};
	mem_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	auto succ = vmaCreateImage(resource_data.vma_allocator,
							   &image_create_info,
							   &mem_info,
							   &texture.image,
							   &texture.allocation,
							   nullptr);
	check_vk(succ);
	infra::SetResourceName(
		device_data.device, VK_OBJECT_TYPE_IMAGE, (uint64_t)texture.image, creation.name);
	VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	info.image = texture.image;
	ToVKEnum(creation.type, info.viewType);
	info.format = creation.format;
	if (utility::HasDepthOrStencil(creation.format))
	{
		info.subresourceRange.aspectMask =
			utility::HasDepth(creation.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
	}
	else
	{
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.layerCount = 1;
	succ = vkCreateImageView(
		device_data.device, &info, resource_data.allocation_callback, &texture.view);
	check_vk(succ);
	infra::SetResourceName(
		device_data.device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)texture.view, creation.name);
	texture.layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

ResourceHandle CreateVkTexture(const DeviceData &device_data,
							   RuntimeLoopData &rl_data,
							   const TextureCreation &creation,
							   ResourceData &resource_data)
{
	Texture *texture =
		infra::AllocResource<Texture>(resource_data, ResourceType::Texture, creation.name);
	CreateVkTextureInner(device_data, creation, resource_data, texture->handle, *texture);
	if (creation.initial_data)
	{
		VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		uint32_t image_size = creation.width * creation.height * 4;
		buffer_info.size = image_size;

		VmaAllocationCreateInfo memory_info{};
		memory_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
		memory_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocationInfo allocation_info{};
		VkBuffer staging_buffer;
		VmaAllocation staging_allocation;
		check_vk(vmaCreateBuffer(resource_data.vma_allocator,
								 &buffer_info,
								 &memory_info,
								 &staging_buffer,
								 &staging_allocation,
								 &allocation_info));

		// Copy buffer_data
		void *destination_data;
		vmaMapMemory(resource_data.vma_allocator, staging_allocation, &destination_data);
		memcpy(destination_data, creation.initial_data, static_cast<size_t>(image_size));
		vmaUnmapMemory(resource_data.vma_allocator, staging_allocation);

		// Execute command buffer
		VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CommandBuffer *command_buffer = infra::GetInstantCommandBuffer(rl_data);
		vkBeginCommandBuffer(command_buffer->vk_command_buffer, &beginInfo);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {creation.width, creation.height, creation.depth};

		// Transition
		infra::TransitionImageLayout(command_buffer->vk_command_buffer,
									 texture->image,
									 texture->format,
									 VK_IMAGE_LAYOUT_UNDEFINED,
									 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
									 false);
		// Copy
		vkCmdCopyBufferToImage(command_buffer->vk_command_buffer,
							   staging_buffer,
							   texture->image,
							   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							   1,
							   &region);
		// Transition
		infra::TransitionImageLayout(command_buffer->vk_command_buffer,
									 texture->image,
									 texture->format,
									 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
									 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
									 false);

		vkEndCommandBuffer(command_buffer->vk_command_buffer);

		// Submit command buffer
		VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command_buffer->vk_command_buffer;

		vkQueueSubmit(device_data.queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device_data.queue);

		vmaDestroyBuffer(resource_data.vma_allocator, staging_buffer, staging_allocation);

		// TODO: free command buffer
		vkResetCommandBuffer(command_buffer->vk_command_buffer,
							 VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		texture->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	return texture->handle;
}

void DestroyVkTexture(const ResourceHandle &handle,
					  const DeviceData &device_data,
					  ResourceData &resource_data)
{
	Texture *tex = infra::Access<Texture>(resource_data, handle);
	if (tex)
	{
		vkDestroyImageView(device_data.device, tex->view, device_data.allocation_callback);
		vmaDestroyImage(resource_data.vma_allocator, tex->image, tex->allocation);
	}
	infra::ReleaseResourceBase(resource_data, tex);
}

} // namespace cloud::vulkan