#pragma once

#include "../source/Texture.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <d3d12.h>


class TextureManager {
private:
    ID3D12Device* device_ = nullptr;
    ID3D12DescriptorHeap* srvDescriptorHeap_ = nullptr;
    ID3D12GraphicsCommandList* commandList_ = nullptr;
    ID3D12CommandQueue* commandQueue_ = nullptr;

    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;

    Microsoft::WRL::ComPtr<ID3D12Resource> whiteTextureResource;
    D3D12_GPU_DESCRIPTOR_HANDLE whiteTextureHandle = { 0 };

public:
    ~TextureManager() {
        if (device_) { device_ = nullptr; }
        if (srvDescriptorHeap_) { srvDescriptorHeap_ = nullptr; }
        if (commandList_) { commandList_ = nullptr; }
    }

    void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvDescriptorHeap, ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* commandQueue);

    void LoadAllFromFolder(const std::string& folderPath);

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(const std::string& name) const;

    std::vector<std::string>GetTextureNames() const;

    void CreateWhiteDummyTexture(ID3D12Device* device, ID3D12DescriptorHeap* srvDescriptorHeap);

    // ハンドル取得用
    D3D12_GPU_DESCRIPTOR_HANDLE GetWhiteTextureHandle() { return whiteTextureHandle; }

    uint32_t GetSRVIndex()const;
    void AddSRVIndex();
};
