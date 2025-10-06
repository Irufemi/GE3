#include "TitleScene.h"

#include "../SceneManager.h"
#include "../SceneName.h"
#include "../../engine/IrufemiEngine.h"


// 初期化
void TitleScene::Initialize(IrufemiEngine* engine) {
    (void)engine;

    engine_ = engine;

}

// 更新
void TitleScene::Update() {

    //エンターキーが押されていたら
    if (engine_->GetInputManager()->IsKeyPressed(VK_RETURN)) {
        if (g_SceneManager) {
            g_SceneManager->Request(SceneName::inGame);
        }
    }

}

// 更新
void TitleScene::Draw() {

}