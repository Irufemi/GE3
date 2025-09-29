#include "ParticleClass.h"

#include "Math.h"

#include <algorithm>

void ParticleClass::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap, Camera* camera, TextureManager* textureManager, DebugUI* ui, const std::string& textureName) {

    this->camera_ = camera;
    this->textureManager_ = textureManager;
    this->ui_ = ui;

    randomEngine.seed(seedGenerator());

    // InstancingようのParticleForGPUリソースを作る
    instancingResource = CreateBufferResource(device.Get(), sizeof(ParticleForGPU) * kNumMaxInstance);
    // 書き込むためのアドレスを取得
    instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));

    numInstance = 0; // 描画すべきインスタンス数

    // 単位行列を書きこんでおく
    for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
        if (particles[index].lifeTime <= particles[index].currentTime) { // 生存時間を過ぎていたら更新せず描画対象にしない
            continue;
        }

        // 位置と速度を[-1,1]でランダムに初期化
        particles[index] = MakeNewParticle(randomEngine);
        instancingData[index].WVP = Math::MakeIdentity4x4();
        instancingData[index].world = Math::MakeIdentity4x4();
        instancingData[index].color = particles[index].color;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC instancingDesc{};
    instancingDesc.Format = DXGI_FORMAT_UNKNOWN;
    instancingDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    instancingDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    instancingDesc.Buffer.FirstElement = 0;
    instancingDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    instancingDesc.Buffer.NumElements = kNumMaxInstance;
    instancingDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
    const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    textureManager_->AddSRVIndex();
    instancingSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, textureManager_->GetSRVIndex());
    instancingSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, textureManager_->GetSRVIndex());
    device->CreateShaderResourceView(instancingResource.Get(), &instancingDesc, instancingSrvHandleCPU);

    // D3D12ResourceUtilを生成
    resource_ = std::make_unique<D3D12ResourceUtilParticle>();

    //左下
    resource_->vertexDataList_.push_back({ { -0.5f,-0.5f,0.0f,1.0f }, { 0.0f,1.0f } });
    //左上
    resource_->vertexDataList_.push_back({ { -0.5f,0.5f,0.0f,1.0f  }, { 0.0f,0.0f} });
    //右下
    resource_->vertexDataList_.push_back({ { 0.5f,-0.5f,0.0f,1.0f }, { 1.0f,1.0f } });
    //右上
    resource_->vertexDataList_.push_back({ { 0.5f,0.5f,0.0f,1.0f }, { 1.0f,0.0f } });

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

    resource_->indexBufferView_ = D3D12_INDEX_BUFFER_VIEW{};
    //リソースの先頭のアドレスから使う
    resource_->indexBufferView_.BufferLocation = resource_->indexResource_->GetGPUVirtualAddress();
    //使用するリソースのサイズ
    resource_->indexBufferView_.SizeInBytes = sizeof(uint32_t) * static_cast<UINT>(resource_->indexDataList_.size());
    //インデックスはint32_tとする
    resource_->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

    ///IndexResourceにデータを書き込む

    //インデックスリソースにデータを書き込む

    std::copy(resource_->indexDataList_.begin(), resource_->indexDataList_.end(), resource_->indexData_);

    //マテリアル

    resource_->materialData_->color = { 1.0f,1.0f,1.0f,1.0f };
    resource_->materialData_->enableLighting = true;
    resource_->materialData_->hasTexture = true;
    resource_->materialData_->lightingMode = 2;
    resource_->materialData_->uvTransform = Math::MakeIdentity4x4();

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

void ParticleClass::Update(const char* particleName) {

    // 速度を反映させる
    for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
        particles[index].transform.translate += particles[index].velocity * kDeltatime;
    }

#ifdef _DEBUG
    std::string name = std::string("Particle: ") + particleName;

    //ImGui

    //ウィンドウを作り出す
    ImGui::Begin(name.c_str());

    ui_->DebugMaterialBy3D(resource_->materialData_);

    ui_->DebugUvTransform(resource_->uvTransform_);

    if (ImGui::CollapsingHeader("InstanceTransform")) {

        for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
            char buf[2];
            buf[0] = '0' + static_cast<char>(index);
            buf[1] = '\0';
            ui_->TextTransform(particles[index].transform, buf);
        }
    }

    //入力終了
    ImGui::End();

#endif // _DEBUG

    for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
        Matrix4x4 worldMatrix = Math::MakeAffineMatrix(particles[index].transform.scale, particles[index].transform.rotate, particles[index].transform.translate);
        Matrix4x4 worldViewProjectionMatrix = Math::Multiply(worldMatrix, Math::Multiply(camera_->GetViewMatrix(), camera_->GetPerspectiveFovMatrix()));
        instancingData[index].WVP = worldViewProjectionMatrix;
        instancingData[index].world = worldMatrix;
        instancingData[index].color = particles[index].color;
    }

    resource_->materialData_->uvTransform = Math::MakeAffineMatrix(resource_->uvTransform_.scale, resource_->uvTransform_.rotate, resource_->uvTransform_.translate);

}

Particle ParticleClass::MakeNewParticle(std::mt19937& randomEngine) {
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
    std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
    Particle particle;
    particle.transform.scale = { 1.0f,1.0f,1.0f };
    particle.transform.rotate = { 0.0f,0.0f,0.0f };
    particle.transform.translate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
    particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
    particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine) ,1.0f };
    particle.lifeTime = distTime(randomEngine);
    particle.currentTime = 0.0f;

    return particle;
}