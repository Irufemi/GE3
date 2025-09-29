#pragma once

#include <d3d12.h>
#include <string>
#include "../camera/Camera.h"
#include "../source/D3D12ResourceUtil.h"
#include "../manager/DebugUI.h"
#include <wrl.h>
#include <memory>

//前方宣言

class DrawManager;
class TextureManager;

//==========================
// objが配布されているサイト
// https://quaternius.com/
// 使用する場合はライセンスがCCOのものを利用する
// https://creativecommons.org/publicdomain/zero/1.0/deed.ja
//==========================

class ObjClass {
protected: //メンバ変数

    ObjModel objModel_;

    std::vector<std::unique_ptr<Texture>> textures_;

    std::vector<std::unique_ptr<D3D12ResourceUtil>> resources_;

#pragma region 外部参照

    Camera* camera_ = nullptr;

    DebugUI* ui_ = nullptr;

#pragma endregion


public: //メンバ関数

    //デストラクタ
    ~ObjClass() = default;

    //初期化
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera* camera, ID3D12DescriptorHeap* srvDescriptorHeap, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, DebugUI* ui, TextureManager* textureManager, const std::string& filename = "plane.obj");

    void Update(const char* objName = " ");

    void Draw(DrawManager* drawManager, D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect);

};

