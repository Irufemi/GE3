#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <memory>
#include <array>
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "../PSOManager.h"

// 前方宣言
class Log;

class DirectXCommon {

public:
    DirectXCommon() = default;
    ~DirectXCommon() = default;

    void Finalize();


    // 1つ目の塊でやってること
    void First(HWND hwnd, int32_t w, int32_t h);

    // 2つ目の塊でやってること
    void Second();

    // 3つ目の塊でやってること
    void Third();

    void SetLog(Log* log) { log_ = log; }

public: // ゲッター

    ID3D12Device* GetDevice() { return this->device_.Get(); }
    ID3D12CommandQueue* GetCommandQueue() { return this->commandQueue_.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() { return this->commandAllocator_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() { return this->commandList_.Get(); }
    IDXGISwapChain4* GetSwapChain() { return this->swapChain_.Get(); }
    ID3D12Fence* GetFence() { return this->fence_.Get(); }
    HANDLE& GetFenceEvent() { return this->fenceEvent_; }
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() { return this->srvDescriptorHeap_.Get(); }
    ID3D12DescriptorHeap* GetDsvDescriptorHeap() { return this->dsvDescriptorHeap_.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE& GetRtvHandles(UINT index) { return this->rtvHandles_[index]; }
    ID3D12Resource* GetSwapChainResources(UINT index) { return this->swapChainResources_[index].Get(); }
    D3D12_VIEWPORT& GetViewport() { return this->viewport_; };
    D3D12_RENDER_TARGET_VIEW_DESC& GetRtvDesc() { return this->rtvDesc_; }
    D3D12_RECT& GetScissorRect() { return this->scissorRect_; };
    HWND& GetHwnd() { return this->hwnd_; }
    DXGI_SWAP_CHAIN_DESC1& GetSwapChainDesc() { return this->swapChainDesc_; }
    ID3D12RootSignature* GetRootSignature() { return this->rootSignature_.Get(); }
    uint64_t& GetFenceValue() { return this->fenceValue_; }
    int32_t& GetClientWidth() { return this->clientWidth_; }
    int32_t& GetClientHeight() { return this->clientHeight_; }
    PSOManager* GetPSOManager() { return psoManager_.get(); }

private:

    // --- Window ---

    HWND hwnd_{};

    // 画面横幅
    int32_t clientWidth_ = 1280;

    // 画面縦幅
    int32_t clientHeight_ = 720;

    //ビューポート
    D3D12_VIEWPORT viewport_ = D3D12_VIEWPORT{};

    //シザー矩形
    D3D12_RECT scissorRect_ = D3D12_RECT{};

    // --- D3D Device & Core ---

    Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

    // --- SwapChain & Render Targets ---

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2] = { nullptr };

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2]{};

    // --- Descriptor Heaps ---

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;

    // --- Depth & Pipeline State ---

    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

    // --- Synchronization --

    Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;

    uint64_t fenceValue_ = 0;

    HANDLE fenceEvent_ = nullptr;

    // Log(ポインタ参照)
    Log* log_ = nullptr;

    // ★追加：PSO 管理インスタンス
    std::unique_ptr<PSOManager> psoManager_ = nullptr;

    //DXGIファクトリー
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
};