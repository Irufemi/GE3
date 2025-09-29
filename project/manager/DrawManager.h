#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <array>
#include <wrl.h>

// 前方宣言
class Sprite;
class SphereClass;
class ObjClass;
class ParticleClass;
class D3D12ResourceUtil;

//描画のCommandListを積む順番
// Viewport → RootSignature → Pipeline → Topology → Buffers → CBV → SRV → Draw

class DrawManager{
private:
    ID3D12GraphicsCommandList* commandList_ = nullptr;
    ID3D12CommandQueue* commandQueue_ = nullptr;
    IDXGISwapChain4* swapChain_ = nullptr;
    ID3D12Fence* fence_ = nullptr;
    HANDLE fenceEvent_ = nullptr;
    ID3D12CommandAllocator* commandAllocator_ = nullptr;
    ID3D12DescriptorHeap* srvDescriptorHeap_ = nullptr;
    ID3D12RootSignature* rootSignature_ = nullptr;
    ID3D12PipelineState* graphicsPipelineState_ = nullptr;

public: //メンバ関数
    void Initialize(
        ID3D12GraphicsCommandList* commandList,
        ID3D12CommandQueue* commandQueue,
        IDXGISwapChain4* swapChain,
        ID3D12Fence* fence,
        HANDLE &fenceEvent,
        ID3D12CommandAllocator* commandAllocator,
        ID3D12DescriptorHeap* srvDescriptorHeap,
        ID3D12RootSignature* rootSignature,
        ID3D12PipelineState* pipelineState
    );

    void Finalize();

    void PreDraw(
        ID3D12Resource* backBufferResource,
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
        ID3D12DescriptorHeap* dsvDescriptorHeap,
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
        std::array<float, 4> clearColor = { 0.1f, 0.25f, 0.5f, 1.0f },
        float clearDepth = 1.0f,
        uint8_t clearStencil = 0
    );

    void PostDraw(
        ID3D12Resource* backBufferResource,
        uint64_t& fenceValue
    );

    void DrawTriangle(
        D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
        ID3D12Resource* materialResource,
        ID3D12Resource* wvpResource,
        ID3D12Resource* directionalLightResource,
        D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU
    );

    void DrawSprite(
        D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        Sprite* sprite
    );

    void DrawSphere(
        D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        SphereClass* sphere
    );

    void DrawParticle(
        D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        ParticleClass* resource
    );

    void DrawByIndex(
        D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        D3D12ResourceUtil* resource
    );

    void DrawByVertex(
        D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        D3D12ResourceUtil* resource
    );
};