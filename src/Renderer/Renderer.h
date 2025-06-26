#pragma once
#include "RHI/CommandQueue.h"
#include "RHI/Swapchain.h"
#include "RHI/CommandBuffer.h"

using namespace RHI;

struct Vertex1
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

class Renderer
{
public:
	static Renderer& GetInstance()
	{
		static Renderer instance;
		return instance;
	}

	Renderer(const Renderer& arg) = delete;
	Renderer& operator=(const Renderer& arg) = delete;

	void Init();
	void Destroy();

	void Render();

	void LoadPipeline();
	void UpdatePipelineDynamicStates();
	void WaitForGpu();

	void RecordAndSubmitCommandBuffers();

	std::shared_ptr<ISwapchain> m_swapchain;
	std::shared_ptr<ICommandQueue> m_commandQueue;
	std::shared_ptr<ICommandBuffer> m_commandBuffer;
	std::shared_ptr<IBuffer> m_buffer;
	ViewportInfo m_viewportInfo;
	ScissorsRect m_scissorsRect;
	std::shared_ptr<ITexture> m_depthStencil;
	std::shared_ptr<ITexture> m_hdrTarget;

private:
	Renderer() {}
};