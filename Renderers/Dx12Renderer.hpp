#pragma once

#define NOMINMAX
#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include "directx/dxgicommon.h"
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>

#include <wrl.h>

#include "Renderers/Renderer.hpp"

class DX12Renderer : public Renderer
{
public:
	DX12Renderer(SDL_Window* aWindow);

	void Initialize() override;
	void Update() override;
	void Resize(unsigned int aWidth, unsigned int aHeight) override;
    virtual const char* Name() override { return "Dx12Renderer"; };

private:
    void WaitForPreviousFrame();
    void GetHardwareAdapter(
        IDXGIFactory1* pFactory,
        IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter);

    static const UINT FrameCount = 2;

    // Pipeline objects.
    Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
    Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargets[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
    UINT mRtvDescriptorSize;

    // App resources.
    Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

    // Synchronization objects.
    UINT mFrameIndex;
    HANDLE mFenceEvent;
    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mFenceValue;
};
