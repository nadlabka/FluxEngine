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

class Renderer1
{
public:
	static Renderer1& GetInstance()
	{
		static Renderer1 instance;
		return instance;
	}

	Renderer1(const Renderer1& arg) = delete;
	Renderer1& operator=(const Renderer1& arg) = delete;

	void Init();
	void Destroy();

	void Render();

	void LoadPipeline();
	void UpdatePipelineDynamicStates();
	void PopulateCommandList();
	void WaitForGpu();

	std::wstring GetAssetFullPath(LPCWSTR assetName)
	{
		return m_assetsPath + assetName;
	}

	std::shared_ptr<ISwapchain> m_swapchain;
	std::shared_ptr<ICommandQueue> m_commandQueue;
	std::shared_ptr<ICommandBuffer> m_commandBuffer;
	std::shared_ptr<IRenderPipeline> m_renderPipeline;
	std::shared_ptr<IBuffer> m_buffer;
	ViewportInfo m_viewportInfo;
	ScissorsRect m_scissorsRect;
	std::shared_ptr<ITexture> m_depthStencil;

	std::wstring m_assetsPath;

private:
	Renderer1() {}
};