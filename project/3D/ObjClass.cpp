#include "ObjClass.h"

#include "../source/Texture.h"
#include "../function/Function.h"
#include "../function/Math.h"
#include "../manager/DrawManager.h"
#include "../manager/TextureManager.h"

void ObjClass::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera* camera, ID3D12DescriptorHeap* srvDescriptorHeap, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, DebugUI* ui, TextureManager* textureManager, const std::string& filename) {

    this->camera_ = camera;
    this->ui_ = ui;

    objModel_ = LoadObjFileM("resources/obj", filename);

    textures_.clear();
    resources_.clear();

    for (const auto& mesh : objModel_.meshes) {

        auto res = std::make_unique<D3D12ResourceUtil>();

        // 頂点バッファ
        res->vertexResource_ = CreateBufferResource(device.Get(), sizeof(VertexData) * mesh.vertices.size());
        res->vertexBufferView_ = D3D12_VERTEX_BUFFER_VIEW{};
        res->vertexBufferView_.BufferLocation = res->vertexResource_->GetGPUVirtualAddress();
        res->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * mesh.vertices.size());
        res->vertexBufferView_.StrideInBytes = sizeof(VertexData);

        res->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&res->vertexData_));
        res->vertexDataList_ = mesh.vertices;
        std::memcpy(res->vertexData_, mesh.vertices.data(), sizeof(VertexData) * mesh.vertices.size());

        // マテリアル
        res->materialResource_ = CreateBufferResource(device.Get(), sizeof(Material));
        res->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&res->materialData_));
        res->materialData_->color = mesh.material.color;
        res->materialData_->enableLighting = mesh.material.enableLighting;
        res->materialData_->hasTexture = true;
        res->materialData_->lightingMode = 2;
        res->materialData_->uvTransform = mesh.material.uvTransform; // すでに行列

        // WVP
        res->transformationMatrix_.world = Math::MakeAffineMatrix(res->transform_.scale, res->transform_.rotate, res->transform_.translate);
        res->transformationMatrix_.WVP = Math::Multiply(res->transformationMatrix_.world, Math::Multiply(camera_->GetViewMatrix(), camera_->GetPerspectiveFovMatrix()));
        res->transformationResource_ = CreateBufferResource(device.Get(), sizeof(TransformationMatrix));
        res->transformationResource_->Map(0, nullptr, reinterpret_cast<void**>(&res->transformationData_));
        *res->transformationData_ = { res->transformationMatrix_.WVP, res->transformationMatrix_.world };

        // テクスチャ
        auto tex = std::make_unique<Texture>();
        if (!mesh.material.textureFilePath.empty()) {
            tex->Initialize(mesh.material.textureFilePath, device.Get(), srvDescriptorHeap, commandList);
            res->textureHandle_ = tex->GetTextureSrvHandleGPU();
        }
        else if(!res->textureHandle_.ptr) {
            res->materialData_->hasTexture = false;
            // ダミー（白）テクスチャのSRVハンドルを取得
            res->textureHandle_ = textureManager->GetWhiteTextureHandle();
        }

        // ライト
        res->directionalLightResource_ = CreateBufferResource(device.Get(), sizeof(DirectionalLight));
        res->directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&res->directionalLightData_));
        res->directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
        res->directionalLightData_->direction = { 0.0f,-1.0f,0.0f, };
        res->directionalLightData_->intensity = 1.0f;

        textures_.push_back(std::move(tex));
        resources_.push_back(std::move(res));
    }

}

void ObjClass::Update(const char* objName) {
#ifdef _DEBUG
    std::string name = std::string("Obj: ") + objName;
    ImGui::Begin(name.c_str());

    for (size_t i = 0; i < resources_.size(); ++i) {
        auto& res = resources_[i];
        std::string meshLabel = "Mesh[" + std::to_string(i) + "]";
        if (ImGui::TreeNode(meshLabel.c_str())) {
            ui_->DebugTransform(res->transform_);
            ui_->DebugMaterialBy3D(res->materialData_);
            ui_->DebugDirectionalLight(res->directionalLightData_);
            ui_->DebugUvTransform(res->uvTransform_);
            ImGui::TreePop();
        }
    }
    ImGui::End();
#endif

    for (auto& res : resources_) {
        res->transformationMatrix_.world = Math::MakeAffineMatrix(res->transform_.scale, res->transform_.rotate, res->transform_.translate);
        res->transformationMatrix_.WVP = Math::Multiply(res->transformationMatrix_.world, Math::Multiply(camera_->GetViewMatrix(), camera_->GetPerspectiveFovMatrix()));
        *res->transformationData_ = { res->transformationMatrix_.WVP, res->transformationMatrix_.world };
        res->materialData_->uvTransform = Math::MakeAffineMatrix(res->uvTransform_.scale, res->uvTransform_.rotate, res->uvTransform_.translate);
        res->directionalLightData_->direction = Math::Normalize(res->directionalLightData_->direction);
    }
}

void ObjClass::Draw(DrawManager* drawManager, D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect) {
    for (auto& res : resources_) {
        drawManager->DrawByVertex(viewport, scissorRect, res.get());
    }
}
