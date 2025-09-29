#pragma once

#include <Windows.h>

#include <vector>
#include <d3d12.h>
#include <cstdint>
#include <wrl.h>

#include "../math/Matrix4x4.h"
#include "../math/Transform.h"
#include "../math/VertexData.h"
#include "../math/Material.h"
#include "../math/TransformationMatrix.h"
#include "../math/DirectionalLight.h"
#include "../manager/TextureManager.h"
#include "../function/Function.h"

class D3D12ResourceUtil {
public: //メンバ変数

#pragma region Vertex

    // 頂点データリスト(position,tecoord,normal)
    std::vector<VertexData> vertexDataList_{};

    //頂点データ(position,tecoord,normal)
    VertexData* vertexData_ = nullptr;

#pragma endregion

#pragma region Index

    //頂点インデックスリスト
    std::vector<uint32_t> indexDataList_{};

    //頂点インデックス
    uint32_t* indexData_ = nullptr;

#pragma endregion

#pragma region Transform

    // transform(scale,rotate,translate)
    Transform transform_ = {
        {1.0f,1.0f,1.0f},   //scale
        {0.0f,0.0f,0.0f},   //rotate
        {0.0f,0.0f,0.0f}    //translate
    };

    // TransformationMatrix(WVP,world)
    TransformationMatrix transformationMatrix_{};

    // TransformationMatrixData(WVP,world)
    TransformationMatrix* transformationData_ = nullptr;

#pragma endregion

#pragma region Material

    //uvTransform(scale,rotate,translate)
    Transform uvTransform_{
        {1.0f,1.0f,1.0f},
        {0.0f,0.0f,0.0f},
        {0.0f,0.0f,0.0f}
    };

    //マテリアルデータ(color,enableLighting,uvTransform)
    Material* materialData_ = nullptr;

#pragma endregion

#pragma region DirectionalLight

    // directionalLight(color,direction,intensity)
    DirectionalLight* directionalLightData_ = nullptr;

#pragma endregion

#pragma region Texture

    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_ = {};

#pragma endregion

#pragma region Buffer

    //頂点データバッファ
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

    //頂点インデックスバッファ
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

#pragma endregion

#pragma region ID3D12Resource

    // 頂点データ用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource_ = nullptr;
    //　頂点インデックス用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> indexResource_ = nullptr;
    // 色用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResource_ = nullptr;
    // 拡縮回転移動行列用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> transformationResource_ = nullptr;
    // 光用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource_ = nullptr;

#pragma endregion

public: //メンバ関数
    //デストラクタ
    ~D3D12ResourceUtil() {
        char buf[256];
        snprintf(buf, sizeof(buf), "[D3D12ResourceUtil] Destruct: vertex=%p, index=%p, material=%p, trans=%p, light=%p\n",
            vertexResource_.Get(), indexResource_.Get(), materialResource_.Get(), transformationResource_.Get(), directionalLightResource_.Get());
        OutputDebugStringA(buf);

        UnMap();
        if (vertexResource_) { vertexResource_.Reset(); }
        if (indexResource_) { indexResource_.Reset(); }
        if (materialResource_) { materialResource_.Reset(); }
        if (transformationResource_) { transformationResource_.Reset(); }
        if (directionalLightResource_) { directionalLightResource_.Reset(); }
    }


    //ID3D12Resourceを生成する
    void CreateResource(ID3D12Device* device) {
        char buf[256];
        if (!vertexDataList_.empty()) {
            vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * static_cast<size_t>(vertexDataList_.size()));
            snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", vertexResource_.Get(), __FILE__, __LINE__);
            OutputDebugStringA(buf);
        }
        if (!indexDataList_.empty()) {
            indexResource_ = CreateBufferResource(device, sizeof(uint32_t) * static_cast<size_t>(indexDataList_.size()));
            snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", indexResource_.Get(), __FILE__, __LINE__);
            OutputDebugStringA(buf);
        }
        materialResource_ = CreateBufferResource(device, sizeof(Material));
        snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", materialResource_.Get(), __FILE__, __LINE__);
        OutputDebugStringA(buf);
        transformationResource_ = CreateBufferResource(device, sizeof(TransformationMatrix));
        snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", transformationResource_.Get(), __FILE__, __LINE__);
        OutputDebugStringA(buf);
        directionalLightResource_ = CreateBufferResource(device, sizeof(DirectionalLight));
        snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", directionalLightResource_.Get(), __FILE__, __LINE__);
        OutputDebugStringA(buf);
    }

    //バッファへの書き込みを開放
    void Map() {
        if (vertexResource_) {
            vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
        }
        if (indexResource_) {
            indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
        }
        materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
        transformationResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationData_));
        directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    }

    //バッファへの書き込みを閉鎖
    void UnMap() {
        if (vertexResource_) {
            vertexResource_->Unmap(0, nullptr);
        }
        if (indexResource_) {
            indexResource_->Unmap(0, nullptr);
        }
        materialResource_->Unmap(0, nullptr);
        transformationResource_->Unmap(0, nullptr);
        directionalLightResource_->Unmap(0, nullptr);
    }
};

class D3D12ResourceUtilParticle {
public: //メンバ変数

#pragma region Vertex

    // 頂点データリスト(position,tecoord,normal)
    std::vector<VertexData> vertexDataList_{};

    //頂点データ(position,tecoord,normal)
    VertexData* vertexData_ = nullptr;

#pragma endregion

#pragma region Index

    //頂点インデックスリスト
    std::vector<uint32_t> indexDataList_{};

    //頂点インデックス
    uint32_t* indexData_ = nullptr;

#pragma endregion

#pragma region Material

    //uvTransform(scale,rotate,translate)
    Transform uvTransform_{
        {1.0f,1.0f,1.0f},
        {0.0f,0.0f,0.0f},
        {0.0f,0.0f,0.0f}
    };

    //マテリアルデータ(color,enableLighting,uvTransform)
    Material* materialData_ = nullptr;

#pragma endregion

#pragma region Texture

    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_ = {};

#pragma endregion

#pragma region Buffer

    //頂点データバッファ
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

    //頂点インデックスバッファ
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

#pragma endregion

#pragma region ID3D12Resource

    // 頂点データ用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource_ = nullptr;
    //　頂点インデックス用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> indexResource_ = nullptr;
    // 色用定数バッファ
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResource_ = nullptr;

#pragma endregion

public: //メンバ関数
    //デストラクタ
    ~D3D12ResourceUtilParticle() {
        char buf[256];
        snprintf(buf, sizeof(buf), "[D3D12ResourceUtil] Destruct: vertex=%p, index=%p, material=%p\n",
            vertexResource_.Get(), indexResource_.Get(), materialResource_.Get());
        OutputDebugStringA(buf);

        UnMap();
        if (vertexResource_) { vertexResource_.Reset(); }
        if (indexResource_) { indexResource_.Reset(); }
        if (materialResource_) { materialResource_.Reset(); }
    }


    //ID3D12Resourceを生成する
    void CreateResource(ID3D12Device* device) {
        char buf[256];
        if (!vertexDataList_.empty()) {
            vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * static_cast<size_t>(vertexDataList_.size()));
            snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", vertexResource_.Get(), __FILE__, __LINE__);
            OutputDebugStringA(buf);
        }
        if (!indexDataList_.empty()) {
            indexResource_ = CreateBufferResource(device, sizeof(uint32_t) * static_cast<size_t>(indexDataList_.size()));
            snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", indexResource_.Get(), __FILE__, __LINE__);
            OutputDebugStringA(buf);
        }
        materialResource_ = CreateBufferResource(device, sizeof(Material));
        snprintf(buf, sizeof(buf), "Created ID3D12Resource at %p in %s:%d\n", materialResource_.Get(), __FILE__, __LINE__);
        OutputDebugStringA(buf);
    }

    //バッファへの書き込みを開放
    void Map() {
        if (vertexResource_) {
            vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
        }
        if (indexResource_) {
            indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
        }
        materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    }

    //バッファへの書き込みを閉鎖
    void UnMap() {
        if (vertexResource_) {
            vertexResource_->Unmap(0, nullptr);
        }
        if (indexResource_) {
            indexResource_->Unmap(0, nullptr);
        }
        materialResource_->Unmap(0, nullptr);
    }
};
