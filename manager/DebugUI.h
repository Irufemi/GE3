#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>


#include "../source/D3D12ResourceUtil.h"
#include "../math/shape/Sphere.h"

// 前方宣言
class TextureManager;

class DebugUI{
private: // メンバ変数

    // ポインタ参照(非所有)

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    TextureManager* textureManager_;

public: // メンバ関数

    // 初期化
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, const Microsoft::WRL::ComPtr<ID3D12Device>& device,HWND &hwnd, DXGI_SWAP_CHAIN_DESC1 &swapChainDesc, D3D12_RENDER_TARGET_VIEW_DESC &rtvDesc, ID3D12DescriptorHeap* srvDescriptorHeap);

    // TextureManagerをセット
    void SetTextureManager(TextureManager* textureManager) { this->textureManager_ = textureManager; }

    // 終了処理
    void Shutdown();

    // フレーム開始
    void FrameStart();

    // 描画処理に入る前にコマンドを積む
    void QueueDrawCommands();

    // 描画処理が終わったタイミングでコマンドを積む
    void QueuePostDrawCommands();

    // Transform
    void DebugTransform(Transform& transform);

    void TextTransform(Transform& transform, const char* name = "");

    // Material
    void DebugMaterialBy3D(Material* material);

    // Material
    void DebugMaterialBy2D(Material* material);

    // 画像
    void DebugTexture(D3D12ResourceUtil * resource_,int & selectedTextureIndex_);

    // DirectionalLight
    void DebugDirectionalLight(DirectionalLight* directionalLightData);

    // UvTransform
    void DebugUvTransform(Transform& uvTransform);

    // Sphere
    void DebugSphereInfo(Sphere& sphere);
};

