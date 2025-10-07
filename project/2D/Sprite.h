#pragma once

#include <d3d12.h>
#include <vector>
#include <cstdint>
#include "../manager/TextureManager.h"
#include "../manager/DebugUI.h"
#include "../source/D3D12ResourceUtil.h"
#include "../camera/Camera.h"
#include "../math/Vector2.h" 
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

    // 追加: サイズとアンカー
    Vector2 size_{ 640.0f, 360.0f };   // 既存の見た目互換のため初期値を640x360に
    Vector2 anchor_{ 0.0f, 0.0f };     // 左上(0,0) / 中央(0.5,0.5) / 右下(1,1)

public: //メンバ関数
    //デストラクタ
    ~Sprite() = default;

    //初期化
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera* camera, TextureManager* textureManager, DebugUI* ui, const std::string& textureName = "uvChecker.png");
    //更新
    void Update();

    // ゲッター
    D3D12ResourceUtil* GetD3D12Resource() { return this->resource_.get(); }

    // サイズとアンカーの設定
    void SetSize(float width, float height);
    void SetAnchor(float ax, float ay);
    Vector2 GetSize() const { return size_; }
    Vector2 GetAnchor() const { return anchor_; }

    // 位置API（アンカー基準の座標を設定/取得）
    void SetPosition(float x, float y, float z = 0.0f);
    Vector2 GetPosition2D() const;

    // 便利エイリアス
    void SetPositionTopLeft(float x, float y) { SetAnchor(0.0f, 0.0f); SetPosition(x, y); }
    void SetPositionCenter(float x, float y) { SetAnchor(0.5f, 0.5f); SetPosition(x, y); }

};