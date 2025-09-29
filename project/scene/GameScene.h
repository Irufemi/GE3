#pragma once

#include "IScene.h"

#include <memory>

#include "../3D/TriangleClass.h"
#include "../2D/Sprite.h"
#include "../3D/SphereClass.h"
#include "../3D/ObjClass.h"
#include "../3D/ParticleClass.h"
#include "../audio/Bgm.h"
#include "../camera/Camera.h"
#include "../camera/DebugCamera.h"

//BGM
#include <xaudio2.h>

// 前方宣言

class IrufemiEngine;


/// <summary>
/// ゲーム
/// </summary>
class GameScene : public IScene {
private: // メンバ変数

    // カメラ
    std::unique_ptr<Camera> camera = nullptr;

    // デバッグカメラ
    std::unique_ptr<DebugCamera> debugCamera = nullptr;

    /*std::unique_ptr<ObjClass> obj = nullptr;
    bool isActiveObj = false;

    std::unique_ptr<Sprite> sprite = nullptr;
    bool isActiveSprite = false;

    std::unique_ptr<TriangleClass> triangle = nullptr;
    bool isActiveTriangle = false;

    std::unique_ptr<SphereClass> sphere = nullptr;
    bool isActiveSphere = true;

    std::unique_ptr<ObjClass> utashTeapot = nullptr;
    bool isActiveUtashTeapot = false;

    std::unique_ptr<ObjClass> stanfordBunny = nullptr;
    bool isActiveStanfordBunny = false;

    std::unique_ptr<ObjClass> multiMesh = nullptr;
    bool isActiveMultiMesh = false;

    std::unique_ptr<ObjClass> multiMaterial = nullptr;
    bool isActiveMultiMaterial = false;

    std::unique_ptr<ObjClass> suzanne = nullptr;
    bool isActiveSuzanne = false;*/

    std::unique_ptr<ParticleClass> particle = nullptr;
    bool isActiveParticle = false;

    std::unique_ptr<Bgm> bgm = nullptr;

    int loadTexture = false;

    bool debugMode = false;

    // ポインタ参照

    // エンジン
    IrufemiEngine* engine_ = nullptr;


public: // メンバ関数

    // デストラクタ
    ~GameScene() {}

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
};