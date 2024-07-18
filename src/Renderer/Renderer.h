#pragma once
#include <SimpleMath.h>
#include <stdafx.h>

struct Vertex
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

    void Render();

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();

    void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter);

    std::wstring GetAssetFullPath(LPCWSTR assetName);

    bool m_useWarpDevice;

    static const UINT FrameCount = 2;

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    RscPtr<IDXGISwapChain3> m_swapChain;
    RscPtr<ID3D12Device> m_device;
    RscPtr<ID3D12Resource> m_renderTargets[FrameCount];
    RscPtr<ID3D12CommandAllocator> m_commandAllocator;
    RscPtr<ID3D12CommandQueue> m_commandQueue;
    RscPtr<ID3D12RootSignature> m_rootSignature;
    RscPtr<ID3D12DescriptorHeap> m_rtvHeap;
    RscPtr<ID3D12PipelineState> m_pipelineState;
    RscPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;

    RscPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    RscPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    std::wstring m_assetsPath;

private:
    Renderer();

};