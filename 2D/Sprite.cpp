#include "Sprite.h"

#include "../function/Math.h"
#include "../function/Function.h"

#include <algorithm>

void Sprite::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera* camera, TextureManager* textureManager, DebugUI* ui, const std::string& textureName) {

    this->camera_ = camera;
    this->textureManager_ = textureManager;
    this->ui_ = ui;

    resource_ = std::make_unique<D3D12ResourceUtil>();

    //左下
    resource_->vertexDataList_.push_back({ { 0.0f,360.0f,0.0f,1.0f }, { 0.0f,1.0f } });
    //左上
    resource_->vertexDataList_.push_back({ { 0.0f,0.0f,0.0f,1.0f  }, { 0.0f,0.0f} });
    //右下
    resource_->vertexDataList_.push_back({ { 640.0f,360.0f,0.0f,1.0f }, { 1.0f,1.0f } });
    //右上
    resource_->vertexDataList_.push_back({ { 640.0f,0.0f,0.0f,1.0f }, { 1.0f,0.0f } });

    for (uint32_t i = 0; i < static_cast<uint32_t>(resource_->vertexDataList_.size()); ++i) {
        resource_->vertexDataList_[i].normal.x = resource_->vertexDataList_[i].position.x;
        resource_->vertexDataList_[i].normal.y = resource_->vertexDataList_[i].position.y;
        resource_->vertexDataList_[i].normal.z = -1.0f;
    }

    resource_->indexDataList_.push_back(0);
    resource_->indexDataList_.push_back(1);
    resource_->indexDataList_.push_back(2);
    resource_->indexDataList_.push_back(1);
    resource_->indexDataList_.push_back(3);
    resource_->indexDataList_.push_back(2);

    // リソースのメモリを確保
    resource_->CreateResource(device.Get());

    // 書き込めるようにする
    resource_->Map();

    //頂点バッファ

    resource_->vertexBufferView_ = D3D12_VERTEX_BUFFER_VIEW{};

    resource_->vertexBufferView_.BufferLocation = resource_->vertexResource_->GetGPUVirtualAddress();
    resource_->vertexBufferView_.StrideInBytes = sizeof(VertexData);
    resource_->vertexBufferView_.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(resource_->vertexDataList_.size());

    std::copy(resource_->vertexDataList_.begin(), resource_->vertexDataList_.end(), resource_->vertexData_);

    /*頂点インデックス*/

    ///Index用のあれやこれやを作る

    resource_->indexBufferView_ = D3D12_INDEX_BUFFER_VIEW{};
    //リソースの先頭のアドレスから使う
    resource_->indexBufferView_.BufferLocation = resource_->indexResource_->GetGPUVirtualAddress();
    //使用するリソースのサイズはインデックス6つ分のサイズ
    resource_->indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    //インデックスはint32_tとする
    resource_->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

    ///IndexResourceにデータを書き込む
    //インデックスリソースにデータを書き込む

    std::copy(resource_->indexDataList_.begin(), resource_->indexDataList_.end(), resource_->indexData_);

    //マテリアル

    resource_->materialData_->color = { 1.0f,1.0f,1.0f,1.0f };
    resource_->materialData_->enableLighting = false;
    resource_->materialData_->hasTexture = true;
    resource_->materialData_->lightingMode = 2;
    resource_->materialData_->uvTransform = Math::MakeIdentity4x4();

    //wvp

    resource_->transformationMatrix_.world = Math::MakeAffineMatrix(resource_->transform_.scale, resource_->transform_.rotate, resource_->transform_.translate);

    resource_->transformationMatrix_.WVP = Math::Multiply(resource_->transformationMatrix_.world, Math::Multiply(camera_->GetViewMatrix(), camera_->GetOrthographicMatrix()));

    *resource_->transformationData_ = { resource_->transformationMatrix_.WVP,resource_->transformationMatrix_.world };

    resource_->directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
    resource_->directionalLightData_->direction = { 0.0f,-1.0f,0.0f, };
    resource_->directionalLightData_->intensity = 1.0f;

    auto textureNames = textureManager_->GetTextureNames();
    std::sort(textureNames.begin(), textureNames.end());
    if (!textureNames.empty()) {

        resource_->textureHandle_ = textureManager_->GetTextureHandle(textureName);

        // コンボボックス用に selectedIndex を初期化
        auto it = std::find(textureNames.begin(), textureNames.end(), textureName);
        if (it != textureNames.end()) {
            selectedTextureIndex_ = static_cast<int>(std::distance(textureNames.begin(), it));
        } else {
            selectedTextureIndex_ = 0;
        }

    }
}

void Sprite::Update() {

#ifdef _DEBUG
    std::string name = std::string("Sprite: ");

    //ImGui

    //カメラウィンドウを作り出す
    ImGui::Begin(name.c_str());
    
    ui_->DebugTransform(resource_->transform_);
    
    ui_->DebugMaterialBy2D(resource_->materialData_);

    ui_->DebugTexture(resource_.get(), selectedTextureIndex_);

    ui_->DebugUvTransform(resource_->uvTransform_);

    //入力終了
    ImGui::End();

#endif // _DEBUG

    resource_->transformationMatrix_.world = Math::MakeAffineMatrix(resource_->transform_.scale, resource_->transform_.rotate, resource_->transform_.translate);

    resource_->transformationMatrix_.WVP = Math::Multiply(resource_->transformationMatrix_.world, Math::Multiply(camera_->GetViewMatrix(), camera_->GetOrthographicMatrix()));

    *resource_->transformationData_ = { resource_->transformationMatrix_.WVP,resource_->transformationMatrix_.world };

    resource_->materialData_->uvTransform = Math::MakeAffineMatrix(resource_->uvTransform_.scale, resource_->uvTransform_.rotate, resource_->uvTransform_.translate);

    resource_->directionalLightData_->direction = Math::Normalize(resource_->directionalLightData_->direction);

}