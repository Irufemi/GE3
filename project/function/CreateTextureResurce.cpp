#include "Function.h"



/*テクスチャを貼ろう*/

///DirectX12のTextureResourceを作る

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata) {
    //1. metadataを基にResourceの設定
    //2. 利用するHeapの設定
    //3. Resourceを生成する

    /*テクスチャを正しく配置しよう*/

    ///正式な手順

    //before
    //1. TextureデータそのものをCPUに読み込む
    //2. DirectX12のTextureResourceを作る)(MainMemory)
    //3. TextureResourceに1で読んだデータを転送する(WriteToSubresource)

    //after
    //1.Textureデータその物をCPUで読み込む

    //2. DorectX12TextureResourceを作る(VRAM)
    //3. CPUに書き込む用にUploadHeapのResourceを作る(IntermediateResource)
    //4. 3に対してCPUでデータを書き込む
    //5. CommandListに3を2に転送するコマンドを積む
    //6. CommandQueueを使って実行する
    //7. 6の事項完了を待つ

    /*テクスチャを貼ろう*/

    //metadataを基にResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = UINT(metadata.width); //Textureの幅
    resourceDesc.Height = UINT(metadata.height); //Textureの高さ
    resourceDesc.MipLevels = UINT(metadata.mipLevels); //mipmapの数
    resourceDesc.DepthOrArraySize = UINT(metadata.arraySize); // 奥行きor 配列Textureの配列数
    resourceDesc.Format = metadata.format; //TextureのFormat
    resourceDesc.SampleDesc.Count = 1; //サンプリングカウント。1固定。
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); //Textureの次元数。普段使っているのは2次元。

    //利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
    D3D12_HEAP_PROPERTIES heapProperties{};
    //heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; //細かい設定を行う
    // heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
    //heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; //プロセッサの近くに配置

    /*テクスチャを正しく配置しよう*/

    //TexturResourceを作る(VRAM)

    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    /*テクスチャを貼ろう*/
    
    //Resourceの生成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties, //Heapの設定
        D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし。
        &resourceDesc, //Resourceの設定
        //D3D12_RESOURCE_STATE_GENERIC_READ, //初回のResourceState。Textureは基本読むだけ。

        /*テクスチャを正しく配置しよう*/

        D3D12_RESOURCE_STATE_COPY_DEST, //データ転送される設定

        /*テクスチャを貼ろう*/

        nullptr, //Clear最適値。使わないのでnullptr
        IID_PPV_ARGS(resource.GetAddressOf()) //作成するResourceポインタへのポインタ
    );
    assert(SUCCEEDED(hr));
    return resource;
}