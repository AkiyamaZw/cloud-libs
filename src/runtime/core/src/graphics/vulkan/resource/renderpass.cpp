#include "graphics/vulkan/resource/renderpass.h"
#include "core/data_structure/resource_pool.h"
#include "graphics/vulkan/device_data.h"
#include "graphics/vulkan/vulkan_interface.h"

namespace cloud::vulkan
{
using namespace cloud::vulkan::infra;

void CreateVkSwapchainRenderPass(const DeviceData &device_data,
								 RuntimeLoopData &rl_data,
								 WindowData &window_data,
								 ResourceData &resource_data,
								 RenderPass &render_pass)
{
	VkAttachmentDescription color_attach{};
	color_attach.format = window_data.window_surface_format.format;
	color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attach_ref{};
	color_attach_ref.attachment = 0;
	color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	Texture *depth_tex = Access<Texture>(resource_data, resource_data.texture_depth_handle);
	VkAttachmentDescription depth_attach{};
	depth_attach.format = depth_tex->format;
	depth_attach.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attach_ref{};
	depth_attach_ref.attachment = 1;
	depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attach_ref;
	subpass.pDepthStencilAttachment = &depth_attach_ref;

	VkAttachmentDescription attaches[] = {color_attach, depth_attach};
	VkRenderPassCreateInfo rpinfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	rpinfo.attachmentCount = 2;
	rpinfo.pAttachments = attaches;
	rpinfo.subpassCount = 1;
	rpinfo.pSubpasses = &subpass;
	check_vk(vkCreateRenderPass(device_data.device, &rpinfo, nullptr, &render_pass.vk_render_pass));
	SetResourceName(device_data.device,
					VK_OBJECT_TYPE_RENDER_PASS,
					(uint64_t)render_pass.vk_render_pass,
					render_pass.name);
	VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	fb_info.renderPass = render_pass.vk_render_pass;
	fb_info.attachmentCount = 2;
	fb_info.width = window_data.swapchain_width;
	fb_info.height = window_data.swapchain_height;
	fb_info.layers = 1;

	VkImageView fb_attaches[2];
	fb_attaches[1] = depth_tex->view;
	// todo 这个循环说不通
	for (size_t i = 0; i < window_data.swapchain_image_count; i++)
	{
		fb_attaches[0] = window_data.swapchain_image_views[i];
		fb_info.pAttachments = fb_attaches;
		vkCreateFramebuffer(
			device_data.device, &fb_info, nullptr, &window_data.swapchain_framebuffers[i]);
		SetResourceName(device_data.device,
						VK_OBJECT_TYPE_FRAMEBUFFER,
						(uint64_t)window_data.swapchain_framebuffers[i],
						std::format("[frame buffer]_{}_index_{}", render_pass.name, i).c_str());
	}

	render_pass.width = window_data.swapchain_width;
	render_pass.height = window_data.swapchain_height;

	VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	CommandBuffer *cb = GetInstantCommandBuffer(rl_data);
	vkBeginCommandBuffer(cb->vk_command_buffer, &info);
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {render_pass.width, render_pass.height, 1};
	for (size_t i = 0; i < window_data.swapchain_image_count; ++i)
	{
		TransitionImageLayout(cb->vk_command_buffer,
							  window_data.swapchain_images[i],
							  window_data.window_surface_format.format,
							  VK_IMAGE_LAYOUT_UNDEFINED,
							  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
							  false);
	}
	vkEndCommandBuffer(cb->vk_command_buffer);
	VkSubmitInfo sub_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	sub_info.commandBufferCount = 1;
	sub_info.pCommandBuffers = &cb->vk_command_buffer;
	vkQueueSubmit(device_data.queue, 1, &sub_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(device_data.queue);
}

void CreateVkFrameBuffer(const DeviceData &device_data,
						 ResourceData &resource_data,
						 RenderPass &rp,
						 const ResourceHandle *out_textures,
						 const uint32_t num_rt,
						 const ResourceHandle &depth_stencil_tex)
{
	VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	fb_info.renderPass = rp.vk_render_pass;
	fb_info.width = rp.width;
	fb_info.height = rp.height;
	fb_info.layers = 1;

	VkImageView fb_attchs[MaxSwapchainImages + 1]{};
	uint32_t active_attachs = 0;
	for (; active_attachs < num_rt; ++active_attachs)
	{
		Texture *tex = Access<Texture>(resource_data, out_textures[active_attachs]);
		fb_attchs[active_attachs] = tex->view;
	}
	if (depth_stencil_tex.index != ResourcePool::INVALID_NUM)
	{
		Texture *tex = Access<Texture>(resource_data, depth_stencil_tex);
		fb_attchs[active_attachs++] = tex->view;
	}
	fb_info.pAttachments = fb_attchs;
	fb_info.attachmentCount = active_attachs;
	check_vk(vkCreateFramebuffer(device_data.device, &fb_info, nullptr, &rp.vk_frame_buffer));
	SetResourceName(
		device_data.device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)rp.vk_frame_buffer, rp.name);
}

RenderPassOutput FillRenderPassOutput(const RenderPassCreation &creation,
									  ResourceData &resource_data)
{
	RenderPassOutput out;
	out.Reset();
	for (uint32_t i = 0; i < creation.num_render_targets; ++i)
	{
		Texture *tex = Access<Texture>(resource_data, creation.output_textures[i]);
		out.SetColorFormat(tex->format);
	}
	if (creation.depth_stencil_texture.index != ResourcePool::INVALID_NUM)
	{
		Texture *tex = Access<Texture>(resource_data, creation.depth_stencil_texture);
		out.SetDepthFormat(tex->format);
	}
	out.color_operation = creation.color_op;
	out.depth_operation = creation.depth_op;
	out.stencil_operation = creation.stencil_op;
	return out;
}

VkRenderPass CreateVkRenderPassInner(const DeviceData &device_data,
									 ResourceData &resource_data,
									 const RenderPassOutput &output,
									 const char *name)
{
	VkAttachmentDescription color_attachs[8] = {};
	VkAttachmentReference color_attachs_ref[8] = {};
	VkAttachmentLoadOp color_op, depth_op, stencil_op;
	VkImageLayout color_initial, depth_initial;
	switch (output.color_operation)
	{
	case RenderPassOperation::Load:
		color_op = VK_ATTACHMENT_LOAD_OP_LOAD;
		color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		break;
	case RenderPassOperation::Clear:
		color_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		break;
	default:
		color_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
		break;
	}
	switch (output.depth_operation)
	{
	case RenderPassOperation::Load:
		depth_op = VK_ATTACHMENT_LOAD_OP_LOAD;
		depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		break;
	case RenderPassOperation::Clear:
		depth_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		break;
	default:
		depth_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
		break;
	}
	switch (output.stencil_operation)
	{
	case RenderPassOperation::Load:
		stencil_op = VK_ATTACHMENT_LOAD_OP_LOAD;
		break;
	case RenderPassOperation::Clear:
		stencil_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		break;
	default:
		stencil_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		break;
	}
	uint32_t c_index = 0;
	for (; c_index < output.num_color_formats; ++c_index)
	{
		VkAttachmentDescription &color_attach = color_attachs[c_index];
		color_attach.format = output.color_formats[c_index];
		color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attach.loadOp = color_op;
		color_attach.stencilLoadOp = stencil_op;
		color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attach.initialLayout = color_initial;
		color_attach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		auto &color_ref = color_attachs_ref[c_index];
		color_ref.attachment = c_index;
		color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	VkAttachmentDescription depth_attach{};
	VkAttachmentReference depth_ref{};
	if (output.depth_stencil_format != VK_FORMAT_UNDEFINED)
	{
		depth_attach.format = output.depth_stencil_format;
		depth_attach.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attach.loadOp = depth_op;
		depth_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attach.stencilLoadOp = stencil_op;
		depth_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attach.initialLayout = depth_initial;
		depth_attach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_ref.attachment = c_index;
		depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	VkAttachmentDescription attachs[MaxSwapchainImages + 1]{};
	uint32_t active_attach = 0;
	for (; active_attach < output.num_color_formats; ++active_attach)
	{
		attachs[active_attach] = color_attachs[active_attach];
		++active_attach;
	}
	subpass.pDepthStencilAttachment = nullptr;
	uint32_t depth_stencil_count = 0;
	if (output.depth_stencil_format != VK_FORMAT_UNDEFINED)
	{
		attachs[subpass.colorAttachmentCount] = depth_attach;
		subpass.pDepthStencilAttachment = &depth_ref;
		depth_stencil_count = 1;
	}
	VkRenderPassCreateInfo rp_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	rp_info.attachmentCount = (active_attach ? active_attach - 1 : 0) + depth_stencil_count;
	rp_info.pAttachments = attachs;
	rp_info.subpassCount = 1;
	VkRenderPass vk_rp;
	check_vk(vkCreateRenderPass(device_data.device, &rp_info, nullptr, &vk_rp));
	SetResourceName(device_data.device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)vk_rp, name);
	return vk_rp;
}

VkRenderPass GetVkRenderPass(const DeviceData &device_data,
							 ResourceData &resource_data,
							 const RenderPassOutput &output,
							 const char *name)
{
	size_t rp_hash = 0;
	render::hash_combine(rp_hash, (void *)&output);
	auto iter = resource_data.render_pass_cache.find(rp_hash);
	if (iter != resource_data.render_pass_cache.end())
	{
		return iter->second;
	}
	VkRenderPass vk_rp = CreateVkRenderPassInner(device_data, resource_data, output, name);
	resource_data.render_pass_cache.emplace(rp_hash, vk_rp);
	return vk_rp;
}

ResourceHandle CreateVkRenderPass(const RenderPassCreation &creation,
								  const DeviceData &device_data,
								  RuntimeLoopData &rl_data,
								  WindowData &window_data,
								  ResourceData &resource_data)
{

	RenderPass *rp =
		AllocResource<RenderPass>(resource_data, ResourceType::RenderPass, creation.name);
	rp->type = creation.type;
	rp->num_render_targets = creation.num_render_targets;
	rp->dispatch_x = 0;
	rp->dispatch_y = 0;
	rp->dispatch_z = 0;
	rp->vk_frame_buffer = VK_NULL_HANDLE;
	rp->vk_frame_buffer = VK_NULL_HANDLE;
	rp->scale_x = creation.scale_x;
	rp->scale_y = creation.scale_y;
	rp->resize = creation.resize;

	uint32_t index = 0;
	for (; index < creation.num_render_targets; index++)
	{
		Texture *tex = Access<Texture>(resource_data, creation.output_textures[index]);
		rp->width = tex->width;
		rp->height = tex->height;
		rp->out_textures[index] = creation.output_textures[index];
	}
	rp->out_depth = creation.depth_stencil_texture;
	if (creation.type == RenderPassType::SwapChain)
	{
		CreateVkSwapchainRenderPass(device_data, rl_data, window_data, resource_data, *rp);
	}
	else if (creation.type == RenderPassType::Compute)
	{
		// todo not implement now!
	}
	else if (creation.type == RenderPassType::Geometry)
	{
		rp->output = FillRenderPassOutput(creation, resource_data);
		rp->vk_render_pass =
			CreateVkRenderPassInner(device_data, resource_data, rp->output, rp->name);
		CreateVkFrameBuffer(device_data,
							resource_data,
							*rp,
							creation.output_textures,
							creation.num_render_targets,
							creation.depth_stencil_texture);
	}

	return rp->handle;
}

void DestroyVkRenderPass(const ResourceHandle &handle,
						 const DeviceData &device_data,
						 ResourceData &resource_data)
{
	RenderPass *rp = Access<RenderPass>(resource_data, handle);
	if (rp)
	{
		if (rp->num_render_targets)
		{
			vkDestroyFramebuffer(
				device_data.device, rp->vk_frame_buffer, resource_data.allocation_callback);
		}
		if (rp->vk_render_pass)
		{
			vkDestroyRenderPass(
				device_data.device, rp->vk_render_pass, resource_data.allocation_callback);
		}
	}
	ReleaseResourceBase(resource_data, rp);
}

} // namespace cloud::vulkan