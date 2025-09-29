#pragma once

#include "../source/D3D12ResourceUtil.h"
#include "../camera/Camera.h"
#include "../manager/TextureManager.h"
#include "../manager/DebugUI.h"
#include "../math/shape/Particle.h"
#include "../math/shape/ParticleForGPU.h"
#include <wrl.h>
#include <memory>
#include <cstdint>

#include <random>


class ParticleClass{
private: // メンバ変数

    static inline const uint32_t kNumMaxInstance = 10; // 最大インスタンス数

    uint32_t numInstance = 0; // 最大インスタンス数

    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource = nullptr;

    ParticleForGPU* instancingData = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU{};

    D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU{};

    Particle particles[kNumMaxInstance];

    std::unique_ptr<D3D12ResourceUtilParticle> resource_ = nullptr;

    int selectedTextureIndex_ = 0;

    // デルタタイム
    static inline const float kDeltatime = 1.0f / 60.0f;

    std::random_device seedGenerator;
    std::mt19937 randomEngine;
    // ポインタ参照

    Camera* camera_ = nullptr;

    TextureManager* textureManager_ = nullptr;

    DebugUI* ui_ = nullptr;

public: // メンバ関数

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap, Camera* camera, TextureManager* textureManager, DebugUI* ui, const std::string& textureName = "uvChecker.png");

    /// <summary>
    /// 更新
    /// </summary>
    void Update(const char* particleName = "");

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    Particle MakeNewParticle(std::mt19937& randomEngine);

    //ゲッター
    D3D12ResourceUtilParticle* GetD3D12Resource() { return this->resource_.get(); }
    int32_t GetInstanceCount() const { return this->kNumMaxInstance; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetInstancingSrvHandleGPU() const { return instancingSrvHandleGPU; }
};

