#include "ResultScene.h"
#include "../SceneManager.h"
#include "../SceneName.h"
#include "../../engine/IrufemiEngine.h"


// デストラクタ
ResultScene::~ResultScene() {}

// 初期化
void ResultScene::Initialize(IrufemiEngine* engine) {
	(void)engine;
	engine_ = engine;
}

// 更新
void ResultScene::Update() {

	//エンターキーが押されていたら
	if (engine_->GetInputManager()->IsKeyPressed(VK_RETURN)) {
		if (g_SceneManager) {
			g_SceneManager->Request(SceneName::title);
		}
	}
}

// 描画
void ResultScene::Draw() {
}
