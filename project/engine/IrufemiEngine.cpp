#include "IrufemiEngine.h"
#include "../function/Function.h"
#include "../function/GetBackBufferIndex.h"

#include <cassert>
#include <DbgHelp.h>
#include <cstdint>
#include <format>

#include "../math/VertexData.h"
#include "../source/D3D12ResourceUtil.h"

#include "../scene/IScene.h"
#include "../scene/title/TitleScene.h"
#include "../scene/inGame/GameScene.h"
#include "../scene/result/ResultScene.h"
#include "../scene/SceneName.h"

#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

//デストラクタ
IrufemiEngine::~IrufemiEngine() { Finalize(); }

// 初期化
void IrufemiEngine::Initialize(const std::wstring& title, const int32_t& clientWidth, const int32_t& clientHeight) {

    /*テクスチャを貼ろう*/

    ///COMの初期化

    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    assert(SUCCEEDED(hr));

    // AudioManagerの生成・Media Foundationの初期化
    audioManager_ = std::make_unique<AudioManager>();
    audioManager_->StartUp();

    /*CrashHandler*/
    SetUnhandledExceptionFilter(ExportDump);

    // ログを出せるようにする
    log_ = std::make_unique<Log>();
    log_->Initialize();

    dxCommon_ = std::make_unique<DirectXCommon>();
    dxCommon_->SetLog(log_.get());

    dxCommon_->First(title, clientWidth, clientHeight);


    inputManager_ = std::make_unique<InputManager>();
    inputManager_->Initialize();

    // AudioManagerの初期化
    audioManager_->Initialize();

    // "resources"フォルダから音声ファイルをすべてロード
    audioManager_->LoadAllSoundsFromFolder("resources/");

    dxCommon_->Second();

    ui = std::make_unique <DebugUI>();
    ui->Initialize(GetCommandList(), GetDevice(), GetHwnd(), GetSwapChainDesc(), GetRtvDesc(), GetSrvDescriptorHeap());


    drawManager = std::make_unique< DrawManager>();
    drawManager->Initialize(
        GetCommandList(),
        GetCommandQueue(),
        GetSwapChain(),
        GetFence(),
        GetFenceEvent(),
        GetCommandAllocator(),
        GetSrvDescriptorHeap(),
        GetRootSignature()
    );

    textureManager = std::make_unique <TextureManager>();
    textureManager->Initialize(GetDevice(), GetSrvDescriptorHeap(), GetCommandList(), GetCommandQueue());
    textureManager->LoadAllFromFolder("resources/");

    ui->SetTextureManager(textureManager.get());

}

void IrufemiEngine::Finalize() {

    // 入力系の解放
    if (inputManager_) {
        inputManager_.reset();
    }
    // サウンド
    if (audioManager_) {
        audioManager_->Finalize();
        audioManager_.reset();
    }
    // 描画
    if (drawManager) {
        drawManager->Finalize();
        drawManager.reset();
    }
    // UI
    if (ui) {
        ui->Shutdown();
        ui.reset();
    }

    dxCommon_->Finalize();

    // COM解放
    CoUninitialize();
}

namespace {
    constexpr std::array<const char*, static_cast<size_t>(SceneName::CountOfSceneName)> kSceneLabels = {
        "Title", // SceneName::title
        "InGame", // SceneName::inGame
        "Result", // SceneName::result
    };
    static_assert(kSceneLabels.size() == static_cast<size_t>(SceneName::CountOfSceneName), "mismatch");
} // namespace

void IrufemiEngine::Execute() {

    // SceneManager 構築・登録
    
    sceneManager_ = std::make_unique<SceneManager>(this);     // ★エンジンを渡す
    g_SceneManager = sceneManager_.get();

    // シーンを登録

    sceneManager_->Register(SceneName::title, [] { return std::make_unique<TitleScene>(); });
    sceneManager_->Register(SceneName::inGame, [] { return std::make_unique<GameScene>(); });
    sceneManager_->Register(SceneName::result, [] { return std::make_unique<ResultScene>(); });

    // 初期シーン
    sceneManager_->ChangeTo(SceneName::inGame);

    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // 入力
            inputManager_->Update();
            // ImGui
            ui->FrameStart();

#ifdef _DEBUG

            // 　シーン選択UI（Requestで要求を出す）
            ImGui::Begin("Scene Selector");
            int idx = static_cast<int>(g_SceneManager->GetCurrent());
            if (ImGui::Combo("Scene", &idx, kSceneLabels.data(), static_cast<int>(kSceneLabels.size()))) {
                g_SceneManager->Request(static_cast<SceneName>(idx)); // or ChangeTo(...)
            }
            ImGui::End();

#endif // _DEBUG

            // 更新
            sceneManager_->Update();

            // フレーム途中処理
            ProcessFrame();

            // 描画
            sceneManager_->Draw();

            // 終了処理
            EndFrame();
        }
    }
}

// フレーム開始処理
void IrufemiEngine::StartFrame() {

}

// フレーム途中処理
void IrufemiEngine::ProcessFrame() {
    // 描画処理に入る前にImGui::Renderを積む
    ui->QueueDrawCommands();

    //これから書き込むバックバッファのインデックスを取得
    backBufferIndex_ = GetBackBufferIndex(GetSwapChain());

    ///DSVを設定する

    //描画先のRTVとDSVを設定する
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

    drawManager->PreDraw(
        GetSwapChainResources(backBufferIndex_),
        GetRtvHandles(backBufferIndex_),
        GetDsvDescriptorHeap(),
        dsvHandle,
        clearColor_,
        1.0f,
        0
    );


}

// フレーム終了処理
void IrufemiEngine::EndFrame() {

    // 描画後処理

    ui->QueuePostDrawCommands();

    drawManager->PostDraw(
        GetSwapChainResources(backBufferIndex_),
        GetFenceValue()
    );
}

void IrufemiEngine::ApplyPSO() {
    auto* pso = GetPSOManager()->Get(currentBlend_, currentDepth_);
    assert(pso && "PSO is null. Check PSOManager::Initialize and shader blobs.");
    if (pso) { drawManager->BindPSO(pso); }
}

void IrufemiEngine::ApplyParticlePSO() {
    auto* pso = GetPSOManager()->GetParticle(currentBlend_, currentDepth_);
    assert(pso && "Particle PSO is null. Check particle shader setup.");
    if (pso) { drawManager->BindPSO(pso); }
}