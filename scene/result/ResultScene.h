#pragma once
#include "../IScene.h"

class ResultScene : public IScene {
public:
	// デストラクタ
	~ResultScene();
	// 初期化
	void Initialize(IrufemiEngine* engine) override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;

private: // メンバ変数
	IrufemiEngine* engine_ = nullptr;
};
