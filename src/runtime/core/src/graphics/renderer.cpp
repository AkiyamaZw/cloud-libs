#include "graphics/renderer.h"
#include "graphics/device.h"
namespace cloud::render
{
Renderer g_render(RendererCreation{});

Renderer *Renderer::Inst() { return &g_render; }

Renderer::Renderer(const RendererCreation &creation)
	: samplers_(128)
	, gpu_device_(nullptr)
{
}

Renderer::~Renderer() {}

void Renderer::Init() {}

void Renderer::Exit()
{
	samplers_.Shutdown();
	gpu_device_->ShutdownGpuDevice();
	DestroyGpuDevice(gpu_device_);
	gpu_device_ = nullptr;
}

void Renderer::BeginFrame() {}

void Renderer::EndFrame() { gpu_device_->Present(); }

void Renderer::ResizeSwapChain(uint32_t width, uint32_t height) {}

SamplerResource *Renderer::CreateSampler(const SamplerCreation &creation)
{
	SamplerResource *sampler = samplers_.Fetch();
	if (sampler)
	{
		SamplerHandle handle = gpu_device_->CreateSampler(creation);
		sampler->handle = handle;
		sampler->name = creation.name;
	}
	return sampler;
}

void CreateRenderer(GpuCreateParam &param)
{
	Renderer *renderer = Renderer::Inst();
	GpuDevice *device = CreateGpuDevice(param.type_device);
	device->InitGpuDevice(param);
	renderer->gpu_device_ = device;
}

void DestroyRenderer() { Renderer::Inst()->Exit(); }
} // namespace cloud::render