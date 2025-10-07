#pragma once

#include <d3d12.h>
#include <string>
#include <wrl.h>

class Texture {
protected:

    //SRVを生成するDescriptorHeapの場所を決める
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_{};
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
    std::string filePath_;
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
    static uint32_t index_;

    // 読み込んだテクスチャの元サイズ
    uint32_t width_ = 0;
    uint32_t height_ = 0;

public:
    //デストラクタ
    ~Texture() = default;

    //初期化
    void Initialize(const std::string& filePath, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

    //ゲッター

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() { return textureSrvHandleGPU_; }

    // サイズ取得
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    static uint32_t GetStaticSRVIndex() { return index_; }

    static void AddStaticSRVIndex() { index_++; }
};

