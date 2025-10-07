#include "Texture.h"

#include "../function/Function.h"

#include "../externals/DirectXTex/DirectXTex.h"

#include "../externals/DirectXTex/d3dx12.h"


void Texture::Initialize(const std::string& filePath, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {

    index_ += 1;

    this->filePath_ = filePath;
    this->device_ = device;
    this->srvDescriptorHeap_ = srvDescriptorHeap;
    this->commandList_ = commandList;

    /*テクスチャを貼ろう*/

    ///組み合わせて使う
    //textureを読んで転送する
    DirectX::ScratchImage mipImages = LoadTexture(filePath_);
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    // メタデータから元サイズを保持
    width_ = static_cast<uint32_t>(metadata.width);
    height_ = static_cast<uint32_t>(metadata.height);

    textureResource_ = CreateTextureResource(device_.Get(), metadata);
    //UploadTextureData(textureResource, mipImages);

    /*テクスチャを正しく配置しよう*/

    ///コマンドを実行して完了を待つ

    intermediateResource_ = UploadTextureData(textureResource_.Get(), mipImages, device_.Get(), commandList_.Get());

    ///実際にShaderResourceViewを作る

    //metaDataを基にSRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    const uint32_t descriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    (void)descriptorSizeSRV; // 未使用警告抑制
    const uint32_t descriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t descriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    //SRVを生成するDescriptorHeapの場所を決める
    //先頭はImGuiが使っているのでその次を使う
    textureSrvHandleCPU_ = GetCPUDescriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV, index_);
    textureSrvHandleGPU_ = GetGPUDescriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV, index_);

    //SRVの生成
    device_->CreateShaderResourceView(textureResource_.Get(), &srvDesc, textureSrvHandleCPU_);
}