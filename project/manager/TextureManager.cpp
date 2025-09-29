#include "TextureManager.h"

#include "../function/Function.h"

/*テクスチャを貼ろう*/

#include "../externals/DirectXTex/DirectXTex.h"

/*テクスチャを正しく配置しよう*/

///事前準備

#include "../externals/DirectXTex/d3dx12.h"

#include <filesystem>
#include <algorithm>
#include <string>
#include <memory>
#include <cassert>
#include <wrl.h>
#include <windows.h> 

uint32_t Texture::index_ = 0;

void TextureManager::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvDescriptorHeap, ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* commandQueue) {
    device_ = device;
    srvDescriptorHeap_ = srvDescriptorHeap;
    commandList_ = commandList;
    commandQueue_ = commandQueue;

    CreateWhiteDummyTexture(device_, srvDescriptorHeap_);
}

void TextureManager::LoadAllFromFolder(const std::string& folderPath) {
    namespace fs = std::filesystem;

    // --- 一時コマンドアロケータとコマンドリストを作成 ---
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
    HRESULT hr = device_->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&allocator)
    );
    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
    hr = device_->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        allocator.Get(),
        nullptr,
        IID_PPV_ARGS(&uploadCommandList)
    );
    assert(SUCCEEDED(hr));

    // --- テクスチャの一括ロード ---
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

            if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp") {
                std::string filename = entry.path().filename().string();

                // 既にロード済みならスキップ
                if (textures_.count(filename) > 0) {
                    continue;
                }

                std::string fullPath = folderPath + "/" + filename;

                auto texture = std::make_shared<Texture>();
                texture->Initialize(fullPath, device_, srvDescriptorHeap_, uploadCommandList.Get());
                textures_[filename] = texture;
            }
        }
    }

    // --- コマンドリストを閉じる ---
    hr = uploadCommandList->Close();
    assert(SUCCEEDED(hr));

    // --- GPUに送信 ---
    ID3D12CommandList* cmdLists[] = { uploadCommandList.Get() };
    commandQueue_->ExecuteCommandLists(1, cmdLists);

    // --- フェンスで同期 ---
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));

    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    assert(fenceEvent);

    uint64_t fenceValue = 1;
    commandQueue_->Signal(fence.Get(), fenceValue);
    if (fence->GetCompletedValue() < fenceValue) {
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
    CloseHandle(fenceEvent);

    // --- コマンドオブジェクトは関数終了時に自動解放される ---
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureHandle(const std::string& name) const {
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        return it->second->GetTextureSrvHandleGPU();
    }
    return {};
}

std::vector<std::string> TextureManager::GetTextureNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : textures_) {
        names.push_back(name);
    }
    return names;
}

void TextureManager::CreateWhiteDummyTexture(ID3D12Device* device, ID3D12DescriptorHeap* srvDescriptorHeap) {
    // 2x2の白画像（全画素RGBA=255,255,255,255）
    uint32_t whitePixels[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = 2; // ★ 2x2に
    texDesc.Height = 2;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // ★ SRGBフォーマット
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

    // テクスチャリソース生成
    device->CreateCommittedResource(
        &heapProp, D3D12_HEAP_FLAG_NONE,
        &texDesc, D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, IID_PPV_ARGS(&whiteTextureResource)
    );

    // アップロードリソース作成
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(whitePixels));
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadResource)
    );

    // ピクセルデータコピー
    uint8_t* mapped = nullptr;
    uploadResource->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    memcpy(mapped, whitePixels, sizeof(whitePixels));
    uploadResource->Unmap(0, nullptr);

    // ---- ここからコマンドリストでコピー処理が本来は必要 ----
    // ・CopyTextureRegion
    // ・ResourceBarrierでCOPY_DEST→PIXEL_SHADER_RESOURCE
    // ※省略している場合、テクスチャの中身が正しく転送されません（実務では必須）

    // SRVを作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // ★ SRGBフォーマット
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    device->CreateShaderResourceView(whiteTextureResource.Get(), &srvDesc, cpuHandle);

    whiteTextureHandle = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

uint32_t TextureManager::GetSRVIndex()const {
    return Texture::GetStaticSRVIndex();
}

void TextureManager::AddSRVIndex() {
    Texture::AddStaticSRVIndex();
}