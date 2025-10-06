#pragma once

#include "../IScene.h"

/// <summary>
/// タイトル
/// </summary>
class TitleScene : public IScene {
public: // メンバ関数

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(IrufemiEngine* engine) override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;

private: // メンバ変数
    IrufemiEngine* engine_ = nullptr;

};

