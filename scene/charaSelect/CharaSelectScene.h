#pragma once
#include "../IScene.h"

class CharaSelectScene : public IScene {
public:
	// デストラクタ
	~CharaSelectScene();
	// 初期化
	void Initialize(IrufemiEngine* engine) override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;

private: // メンバ変数

	IrufemiEngine* engine_ = nullptr;
};
