#include "CharaSelectScene.h"
#include "../SceneManager.h"
#include "../SceneName.h"
#include "../../engine/IrufemiEngine.h"


// デストラクタ
CharaSelectScene::~CharaSelectScene() {}

// 初期化
void CharaSelectScene::Initialize(IrufemiEngine * engine) {
    engine_ = engine;
}

// 更新
void CharaSelectScene::Update() {

    //エンターキーが押されていたら
    if (engine_->GetInputManager()->IsKeyPressed(VK_RETURN)) {
        if (g_SceneManager) {
            g_SceneManager->Request(SceneName::inGame);
        }
    }
}

// 描画
void CharaSelectScene::Draw() {
}
