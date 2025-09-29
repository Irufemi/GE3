#include "DebugUI.h"

#include <Windows.h>

/*開発のUIを出そう*/

#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <numbers>
#include "../manager/TextureManager.h"

void DebugUI::Initialize(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, const Microsoft::WRL::ComPtr<ID3D12Device>& device, HWND& hwnd, DXGI_SWAP_CHAIN_DESC1& swapChainDesc, D3D12_RENDER_TARGET_VIEW_DESC& rtvDesc, ID3D12DescriptorHeap* srvDescriptorHeap) {

    this->commandList_ = commandList;

    /*開発UIを出そう*/
    //ImGuiの初期化。詳細はさして重要ではないので開設は省略する。
    //こういうもんである
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(
        device.Get(),
        swapChainDesc.BufferCount,
        rtvDesc.Format,
        srvDescriptorHeap,
        srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
    );

}

void DebugUI::FrameStart() {

    /*開発のUIを出そう*/

    ///ImGuiを使う
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

}

void DebugUI::Shutdown() {

    if (commandList_) { commandList_.Reset(); }
    /*開発のUIを出そう*/

    ///ImGuiの終了処理

    //ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
    //こういうもんである。初期化と逆順に行う。
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

}

void DebugUI::QueueDrawCommands() {

    /*開発のUIを出そう*/

    ///ImGuiを使う

    //ImGuiの内部コマンドを生成する
    ImGui::Render();
}

void DebugUI::QueuePostDrawCommands() {

    /*開発のUIを出そう*/

    ///ImGuiを描画する

    //実際のcommandListのImGuiの描画コマンドを積む
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_.Get());

}

// transform
void DebugUI::DebugTransform(Transform& transform) {
    if (ImGui::CollapsingHeader("transform")) {
        ImGui::DragFloat3("scale", &transform.scale.x, 0.05f);
        ImGui::DragFloat3("rotate", &transform.rotate.x, 0.05f);
        ImGui::DragFloat3("translate", &transform.translate.x, 0.05f);
        static bool rotateX = false;
        ImGui::Checkbox("RotateX", &rotateX);
        if (rotateX) {
            transform.rotate.x += static_cast<float>(0.05f / std::numbers::pi);
        }
        static bool rotateY = false;
        ImGui::Checkbox("RotateY", &rotateY);
        if (rotateY) {
            transform.rotate.y += static_cast<float>(0.05f / std::numbers::pi);
        }
        static bool rotateZ = false;
        ImGui::Checkbox("RotateZ", &rotateZ);
        if (rotateZ) {
            transform.rotate.z += static_cast<float>(0.05f / std::numbers::pi);
        }
    }
}

void DebugUI::TextTransform(Transform& transform,const char* name) {
    std::string header = std::string("transform") + name;
    if (ImGui::CollapsingHeader(header.c_str())) {
        ImGui::Text("scale: (%.2f, %.2f, %.2f)", transform.scale.x, transform.scale.y, transform.scale.z);
        ImGui::Text("rotate: (%.2f, %.2f, %.2f)", transform.rotate.x, transform.rotate.y, transform.rotate.z);
        ImGui::Text("translate: (%.2f, %.2f, %.2f)", transform.translate.x, transform.translate.y, transform.translate.z);
    }
}


// Material
void DebugUI::DebugMaterialBy3D(Material* materialData) {
    if (ImGui::CollapsingHeader("material")) {
        ImGui::ColorEdit4("spriteColor", &materialData->color.x);
        bool enableLighting = materialData->enableLighting;
        if (ImGui::Checkbox("enableLighting", &enableLighting)) {
            materialData->enableLighting = enableLighting;
        }
        // lightingMode選択
        const char* items[] = { "NonLighting", "Lambert", "HalfLambert" };
        int currentMode = materialData->lightingMode;
        if (ImGui::Combo("LightingMode", &currentMode, items, IM_ARRAYSIZE(items))) {
            materialData->lightingMode = currentMode;
        }
    }
}

// Material
void DebugUI::DebugMaterialBy2D(Material* materialData) {
    if (ImGui::TreeNodeEx("material")) {
        ImGui::ColorEdit4("spriteColor", &materialData->color.x);
    }
}

// 画像
void DebugUI::DebugTexture(D3D12ResourceUtil* resource, int& selectedTextureIndex) {

    if (ImGui::CollapsingHeader("texture")) {

        std::vector<std::string> textureNames = textureManager_->GetTextureNames();
        std::sort(textureNames.begin(), textureNames.end());

        if (!textureNames.empty()) {
            selectedTextureIndex = std::clamp(selectedTextureIndex, 0, static_cast<int>(textureNames.size()) - 1);
            if (ImGui::BeginCombo("TextureName", textureNames[selectedTextureIndex].c_str())) {
                for (int i = 0; i < textureNames.size(); ++i) {
                    bool isSelected = (i == selectedTextureIndex);
                    if (ImGui::Selectable(textureNames[i].c_str(), isSelected)) {
                        selectedTextureIndex = i;
                        resource->textureHandle_ = textureManager_->GetTextureHandle(textureNames[i]);
                    }
                }
                ImGui::EndCombo();
            }
        } else {
            ImGui::Text("No textures found.");
        }
    }
}

// DirectionalLight
void DebugUI::DebugDirectionalLight(DirectionalLight* directionalLightData) {
    if (ImGui::CollapsingHeader("directionalLight")) {
        ImGui::ColorEdit4("lightColor", &directionalLightData->color.x);
        ImGui::DragFloat3("lightDirection", &directionalLightData->direction.x, 0.01f);
        ImGui::DragFloat("intensity", &directionalLightData->intensity, 0.01f);
    }
}

// UvTransform
void DebugUI::DebugUvTransform(Transform& uvTransform) {
    if (ImGui::CollapsingHeader("uvTransform")) {
        ImGui::DragFloat3("UVTranslate", &uvTransform.translate.x, 0.01f, -10.0f, 10.0f);
        ImGui::DragFloat3("UVScale", &uvTransform.scale.x, 0.01f, -10.0f, 10.0f);
        ImGui::SliderAngle("UVRotate", &uvTransform.rotate.z);
    }
}

// Sphere
void DebugUI::DebugSphereInfo(Sphere& sphere) {
    if (ImGui::CollapsingHeader("info")) {
        ImGui::DragFloat3("Center", &sphere.center.x, 0.01f, -10.0f, 10.0f);
        ImGui::DragFloat("radius", &sphere.radius, 0.01f, -10.0f, 10.0f);
    }
}