#pragma once

// 前方宣言
class IrufemiEngine;

/// <summary>
/// Scene系クラスに継承する基底クラス
/// </summary>
class IScene {
public:
    virtual ~IScene() = default;
    virtual void Initialize(IrufemiEngine* engine) = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
};