#pragma once

#include <cstdint>

#include "../camera/Camera.h"
#include "../manager/TextureManager.h"
#include "../manager/DebugUI.h"
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "../source/D3D12ResourceUtil.h"
#include "../math/shape/Sphere.h"

class SphereClass {
protected: //メンバ変数

    Sphere info_{};

    const float pi_ = 3.141592654f;

    const uint32_t kSubdivision_ = 16;

    //経度分割1つ分の角度 Φd

    const float kLonEvery_ = pi_ * 2.0f / static_cast<float>(kSubdivision_);

    //井戸分割つ分の角度 θd
    const float kLatEvery_ = pi_ / static_cast<float>(kSubdivision_);

    bool isRotateY_ = true;

    // D3D12リソース

    std::unique_ptr<D3D12ResourceUtil> resource_ = nullptr;

    int selectedTextureIndex_ = 0;

    // ポインタ参照

    TextureManager* textureManager_ = nullptr;

    DebugUI* ui_ = nullptr;

    Camera* camera_ = nullptr;

public: //メンバ関数
    // コンストラクタ
    SphereClass() {};

    // デストラクタ
    ~SphereClass() = default;

    // 初期化
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera* camera, TextureManager* textureManager, DebugUI* ui, const std::string& textureName = "uvChecker.png");

    // 更新
    void Update(const char* sphereName = " ");

    D3D12ResourceUtil* GetD3D12Resource() { return this->resource_.get(); }
    void AddRotateY(float value) { this->resource_->transform_.rotate.y += value; }

    // Sphereの情報を取得
    Sphere GetInfo() const { return info_; }
};

