#include "Function.h"

/*テクスチャを貼ろう*/

//void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
//    //Meta情報を取得
//    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
//    //全MipMapについて
//    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
//        //MipMapLevelを指定して各Imageを取得
//        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
//        //Textureに転送
//        HRESULT hr = texture->WriteToSubresource(
//            UINT(mipLevel),
//            nullptr, //全領域へコピー
//            img->pixels, // 元データアドレス
//            UINT(img->rowPitch), //1ラインサイズ
//            UINT(img->slicePitch) //1枚サイズ
//        );
//        assert(SUCCEEDED(hr));
//    }
//}

/*テクスチャを正しく配置しよう*/

[[nodiscard]] //戻り値を破棄しないように
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {
    ///IntermediteResource(中間リソース)
    
    std::vector<D3D12_SUBRESOURCE_DATA> subResources;
    //1. PrepareUploadを利用して、読み込んだデータからDirectX12用のSubresource(サブリソース)の配列を作成する(Subresourceは、MipMapの1枚1枚ぐらいのイメージでいると良い)
    DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subResources);
    //2. Subresourceの数を基に、コピー元となるIntermediateResourceに必要なサイズを計算する
    uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subResources.size()));
    //3. 計算したサイズでIntermediteResourceを作る
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device.Get(), intermediateSize);
    
    ///データ転送をコマンドに積む

    UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subResources.size()), subResources.data());
    
    ///ResourceStateを変更し、IntermediateResourceを返す

    //Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = texture.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
    commandList->ResourceBarrier(1, &barrier);

    return intermediateResource;
}