#pragma once

#include "../source/D3D12ResourceUtil.h"
#include "../camera/Camera.h"
#include "../manager/TextureManager.h"
#include "../manager/DebugUI.h"
#include "../math/shape/Particle.h"
#include "../math/shape/ParticleForGPU.h"
#include "../math/Emitter.h"
#include "../math/AccelerationField.h"
#include "../function/Math.h"
#include <wrl.h>
#include <memory>
#include <cstdint>
#include <numbers>
#include <list>

#include <random>


class ParticleClass {
private: // メンバ変数

    static inline const uint32_t kNumMaxInstance_ = 100; // 最大インスタンス数

    uint32_t numInstance_ = 0; // 最大インスタンス数

    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_ = nullptr;

    ParticleForGPU* instancingData_ = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_{};

    D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};

    std::list<Particle> particles_;

    std::unique_ptr<D3D12ResourceUtilParticle> resource_ = nullptr;

    Matrix4x4 backToFrontMatrix_ = Math::MakeRotateYMatrix({ 0 });
    Matrix4x4 billbordMatrix_{};

    int selectedTextureIndex_ = 0;

    // デルタタイム
    static inline const float kDeltatime_ = 1.0f / 60.0f;

    std::random_device seedGenerator_;
    std::mt19937 randomEngine_;

    Emitter emitter_{};

    AccelerationField accelerationField_{};

    // ポインタ参照

    Camera* camera_ = nullptr;

    TextureManager* textureManager_ = nullptr;

    DebugUI* ui_ = nullptr;

    bool useBillbord_ = true;

    bool isUpdate_ = true;

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

    Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

    std::list<Particle> Emit(const Emitter& emitter, std::mt19937& randomEngine);

    //ゲッター
    D3D12ResourceUtilParticle* GetD3D12Resource() { return this->resource_.get(); }
    int32_t GetInstanceCount() const { return this->numInstance_; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetInstancingSrvHandleGPU() const { return instancingSrvHandleGPU_; }
};

