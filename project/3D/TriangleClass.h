#pragma once

#include <vector>
#include <array>
#include <d3d12.h>

#include "../math/shape/Triangle.h"
#include "../source/D3D12ResourceUtil.h"
#include "../camera/Camera.h"
#include "../manager/TextureManager.h"
#include "../manager/DebugUI.h"
#include <wrl.h>
#include <memory>


class TriangleClass {
protected: //メンバ変数

    Triangle info_{};
    
    std::unique_ptr<D3D12ResourceUtil> resource_ = nullptr;

    int selectedTextureIndex_ = 0;

    // ポインタ参照

    Camera* camera_ = nullptr;

    TextureManager* textureManager_ = nullptr;

    DebugUI* ui_ = nullptr;

public: //メンバ関数
    //デストラクタ
    ~TriangleClass() = default;

    //初期化
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera* camera, TextureManager* textureManager, DebugUI* ui, const std::string& textureName = "uvChecker.png");
    //更新
    void Update(const char* triangleName = "");

    //ゲッター
    D3D12ResourceUtil* GetD3D12Resource() { return this->resource_.get(); }

    // Triangleの情報を取得
    Triangle GetInfo() const { return info_; }
    
};

