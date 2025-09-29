#pragma once

#include <d3d12.h>
#include <vector>
#include <cstdint>
#include "../manager/TextureManager.h"
#include "../manager/DebugUI.h"
#include "../source/D3D12ResourceUtil.h"
#include "../camera/Camera.h"
#include <wrl.h>
#include <memory>

class Sprite {
protected:

    std::unique_ptr<D3D12ResourceUtil> resource_ = nullptr;

    bool isRotateY_ = true;

    int selectedTextureIndex_ = 0;

    Camera* camera_ = nullptr;

    TextureManager* textureManager_ = nullptr;

    DebugUI* ui_ = nullptr;

public: //メンバ関数
    //デストラクタ
    ~Sprite() = default;

    //初期化
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera* camera, TextureManager* textureManager, DebugUI* ui, const std::string& textureName = "uvChecker.png");
    //更新
    void Update();

    // ゲッター
    D3D12ResourceUtil* GetD3D12Resource() { return this->resource_.get(); }

};