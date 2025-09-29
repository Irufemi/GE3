#include <Windows.h>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <array>
#include <d3d12.h>

#include "./engine/IrufemiEngine.h"
#include "externals/imgui/imgui.h"
#include "function/GetBackBufferIndex.h"
#include "./scene/IScene.h"
#include "./scene/TitleScene.h"
#include "./scene/charaSelect/CharaSelectScene.h"
#include "./scene/GameScene.h"
#include "./scene/result/ResultScene.h"
#include "./scene/SceneName.h"
#include "./scene/SceneManager.h"

//クライアント領域のサイズ
const int32_t kClientWidth = 1280;
const int32_t kClientHeight = 720;

// タイトル
const std::wstring kTitle = L"CG3_LE2B_11_スエヒロ_コウイチ";

namespace {
    constexpr std::array<const char*, static_cast<size_t>(SceneName::CountOfSceneName)> kSceneLabels = {
        "Title", // SceneName::title
        "CharaSelect",// SceneName::charaSelect
        "InGame", // SceneName::inGame
        "Result", // SceneName::result
    };
    static_assert(kSceneLabels.size() == static_cast<size_t>(SceneName::CountOfSceneName), "mismatch");
} // namespace

//windowsアプリでのエントリーポint32_tイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {


    std::unique_ptr<IrufemiEngine> engine = std::make_unique<IrufemiEngine>();
    engine->Initialize(kTitle, kClientWidth, kClientHeight);

    // シーンマネジャーの生成
    std::unique_ptr<SceneManager> sm = std::make_unique<SceneManager>(engine.get());     // ★エンジンを渡す
    g_SceneManager = sm.get();

    // シーンを登録

    sm->Register(SceneName::title, [] { return std::make_unique<TitleScene>(); });
    sm->Register(SceneName::charaSelect, [] { return std::make_unique<CharaSelectScene>(); });
    sm->Register(SceneName::inGame, [] { return std::make_unique<GameScene>(); });
    sm->Register(SceneName::result, [] { return std::make_unique<ResultScene>(); });

    // 初期シーン
    sm->ChangeTo(SceneName::inGame);


    //画面の色を設定
    constexpr std::array<float, 4> clearColor = { 0.1f, 0.25f, 0.5f, 1.0f };

    MSG msg{};
    //ウィンドウの×ボタンが押されるまでループ
    while (msg.message != WM_QUIT) {
        //Windowsにメッセージが来てたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {

            //入力情報の更新　
            engine->GetInputManager()->Update();

            //
            engine->GetDebugUI()->FrameStart();

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
            sm->Update();

            // 描画処理に入る前にImGui::Renderを積む
            engine->GetDebugUI()->QueueDrawCommands();

            //これから書き込むバックバッファのインデックスを取得
            UINT backBufferIndex = GetBackBufferIndex(engine->GetSwapChain());

            ///DSVを設定する

            //描画先のRTVとDSVを設定する
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = engine->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

            engine->GetDrawManager()->PreDraw(
                engine->GetSwapChainResources(backBufferIndex),
                engine->GetRtvHandles(backBufferIndex),
                engine->GetDsvDescriptorHeap(),
                dsvHandle,
                clearColor,
                1.0f,
                0
            );

            // 描画
            sm->Draw();

            // 描画後処理

            engine->GetDebugUI()->QueuePostDrawCommands();

            engine->GetDrawManager()->PostDraw(
                engine->GetSwapChainResources(backBufferIndex),
                engine->GetFenceValue()
            );

        }

    }

    return 0;

}