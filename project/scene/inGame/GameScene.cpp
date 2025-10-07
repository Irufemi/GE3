#include "GameScene.h"

#include "../SceneManager.h"
#include "../SceneName.h"
#include "../../engine/IrufemiEngine.h"

#include <algorithm>


// 初期化
void GameScene::Initialize(IrufemiEngine* engine) {

    // 参照したものをコピー
    // エンジン
    this->engine_ = engine;

    camera = std::make_unique <Camera>();
    camera->Initialize(engine_->GetClientWidth(), engine_->GetClientHeight());

    debugCamera = std::make_unique <DebugCamera>();
    debugCamera->Initialize(engine_->GetInputManager(), engine_->GetClientWidth(), engine_->GetClientHeight());
    debugMode = false;

    isActiveObj = false;
    isActiveSprite = false;
    isActiveTriangle = false;
    isActiveSphere = false;
    isActiveStanfordBunny = false;
    isActiveUtashTeapot = false;
    isActiveMultiMesh = false;
    isActiveMultiMaterial = false;
    isActiveSuzanne = false;
    isActiveFence_ = false;
    isActiveParticle = true;


    if (isActiveObj) {
        obj = std::make_unique <ObjClass>();
        obj->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager());
    }
    if (isActiveSprite) {
        sprite = std::make_unique <Sprite>();
        sprite->Initialize(engine_->GetDevice(), camera.get(), engine_->GetTextureManager(), engine_->GetDebugUI());
    }
    if (isActiveTriangle) {
        triangle = std::make_unique <TriangleClass>();
        triangle->Initialize(engine_->GetDevice(), camera.get(), engine_->GetTextureManager(), engine_->GetDebugUI());
    }
    if (isActiveSphere) {
        sphere = std::make_unique <SphereClass>();
        sphere->Initialize(engine_->GetDevice(), camera.get(), engine_->GetTextureManager(), engine_->GetDebugUI());
    }
    if (isActiveStanfordBunny) {
        stanfordBunny = std::make_unique <ObjClass>();
        stanfordBunny->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "bunny.obj");
    }
    if (isActiveUtashTeapot) {
        utashTeapot = std::make_unique <ObjClass>();
        utashTeapot->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "teapot.obj");
    }
    if (isActiveMultiMesh) {
        multiMesh = std::make_unique <ObjClass>();
        multiMesh->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "multiMesh.obj");
    }
    if (isActiveMultiMaterial) {
        multiMaterial = std::make_unique <ObjClass>();
        multiMaterial->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "multiMaterial.obj");
    }
    if (isActiveSuzanne) {
        suzanne = std::make_unique <ObjClass>();
        suzanne->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "suzanne.obj");
    }

    if (isActiveFence_) {
        fence_ = std::make_unique <ObjClass>();
        fence_->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "fence.obj");
    }
    if (isActiveParticle) {
        particle = std::make_unique <ParticleClass>();
        particle->Initialize(engine_->GetDevice(), engine_->GetSrvDescriptorHeap(),camera.get(),engine_->GetTextureManager(),engine_->GetDebugUI(),"circle.png");
    }

    bgm = std::make_unique<Bgm>();
    bgm->Initialize(engine_->GetAudioManager());
    bgm->PlayFirstTrack();
}

// 更新
void GameScene::Update() {

    ImGui::Begin("Activation");
    ImGui::Checkbox("Obj", &isActiveObj);
    ImGui::Checkbox("Sprite", &isActiveSprite);
    ImGui::Checkbox("Triangle", &isActiveTriangle);
    ImGui::Checkbox("Sphere", &isActiveSphere);
    ImGui::Checkbox("Utash Teapot", &isActiveUtashTeapot);
    ImGui::Checkbox("Stanford Bunny", &isActiveStanfordBunny);
    ImGui::Checkbox("MultiMesh", &isActiveMultiMesh);
    ImGui::Checkbox("MultiMaterial", &isActiveMultiMaterial);
    ImGui::Checkbox("Suzanne", &isActiveSuzanne);
    ImGui::Checkbox("Fence", &isActiveFence_);
    ImGui::Checkbox("Particle", &isActiveParticle);
    ImGui::End();

    ImGui::Begin("Texture");
    if (ImGui::Button("allLoadActivate")) {
        engine_->GetTextureManager()->LoadAllFromFolder("resources/");
    }
    ImGui::Checkbox("debugMode", &debugMode);
    ImGui::End();

    if (debugMode) {
        debugCamera->Update();
        camera->SetViewMatrix(debugCamera->GetCamera().GetViewMatrix());
        camera->SetPerspectiveFovMatrix(debugCamera->GetCamera().GetPerspectiveFovMatrix());
    } else {
        camera->Update("Camera");

    }

    // BGM
    bgm->Update();

    // 3D

    if (isActiveObj) {
        if (!obj) {
            obj = std::make_unique<ObjClass>();
            obj->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine_->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager());
        }
        obj->Update("Plane");
    }
    if (isActiveTriangle) {
        if (!triangle) {
            triangle = std::make_unique<TriangleClass>();
            triangle->Initialize(engine_->GetDevice(), camera.get(), engine_->GetTextureManager(), engine_->GetDebugUI());
        }
        triangle->Update();
    }
    if (isActiveSphere) {
        if (!sphere) {
            sphere = std::make_unique<SphereClass>();
            sphere->Initialize(engine_->GetDevice(), camera.get(), engine_->GetTextureManager(), engine_->GetDebugUI());
        }
        sphere->Update();
    }
    if (isActiveUtashTeapot) {
        if (!utashTeapot) {
            utashTeapot = std::make_unique<ObjClass>();
            utashTeapot->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine_->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "teapot.obj");
        }
        utashTeapot->Update("Utash Teapot");
    }
    if (isActiveStanfordBunny) {
        if (!stanfordBunny) {
            stanfordBunny = std::make_unique<ObjClass>();
            stanfordBunny->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine_->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "bunny.obj");
        }
        stanfordBunny->Update("Stanford Bunny");
    }
    if (isActiveMultiMesh) {
        if (!multiMesh) {
            multiMesh = std::make_unique<ObjClass>();
            multiMesh->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine_->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "multiMesh.obj");
        }
        multiMesh->Update("MultiMesh");
    }
    if (isActiveMultiMaterial) {
        if (!multiMaterial) {
            multiMaterial = std::make_unique<ObjClass>();
            multiMaterial->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine_->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "multiMaterial.obj");
        }
        multiMaterial->Update("MultiMaterial");
    }
    if (isActiveSuzanne) {
        if (!suzanne) {
            suzanne = std::make_unique<ObjClass>();
            suzanne->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine_->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "suzanne.obj");
        }
        suzanne->Update("Suzanne");
    }
    if (isActiveFence_) {
        if (!fence_) {
            fence_ = std::make_unique<ObjClass>();
            fence_->Initialize(engine_->GetDevice(), camera.get(), engine_->GetSrvDescriptorHeap(), engine_->GetCommandList(), engine_->GetDebugUI(), engine_->GetTextureManager(), "fence.obj");
        }
        fence_->Update("Fence");
    }
    if (isActiveParticle) {
        if(!particle){
            particle = std::make_unique <ParticleClass>();
            particle->Initialize(engine_->GetDevice(), engine_->GetSrvDescriptorHeap(),camera.get(), engine_->GetTextureManager(), engine_->GetDebugUI());
        }
        particle->Update();
    }

    // 2D

    if (isActiveSprite) {
        if (!sprite) {
            sprite = std::make_unique<Sprite>();
            sprite->Initialize(engine_->GetDevice(), camera.get(), engine_->GetTextureManager(), engine_->GetDebugUI());
        }
        sprite->Update();
    }

   

    //エンターキーが押されていたら
    if (PressedVK(VK_RETURN)) {
        if (g_SceneManager) {
            g_SceneManager->Request(SceneName::result);
        }
    }

}

// 描画
void GameScene::Draw() {

    // 3D

    engine_->SetBlend(BlendMode::kBlendModeNormal);
    engine_->SetDepthWrite(PSOManager::DepthWrite::Enable);
    engine_->ApplyPSO();
    
    if (isActiveObj) {
        obj->Draw(engine_->GetDrawManager(), engine_->GetViewport(), engine_->GetScissorRect());
    }
    if (isActiveTriangle) {
        engine_->GetDrawManager()->DrawByIndex(engine_->GetViewport(), engine_->GetScissorRect(), triangle->GetD3D12Resource());
    }
    if (isActiveSphere) {
        engine_->GetDrawManager()->DrawSphere(engine_->GetViewport(), engine_->GetScissorRect(), sphere.get());
    }
    if (isActiveUtashTeapot) {
        utashTeapot->Draw(engine_->GetDrawManager(), engine_->GetViewport(), engine_->GetScissorRect());
    }
    if (isActiveStanfordBunny) {
        stanfordBunny->Draw(engine_->GetDrawManager(), engine_->GetViewport(), engine_->GetScissorRect());
    }
    if (isActiveMultiMesh) {
        multiMesh->Draw(engine_->GetDrawManager(), engine_->GetViewport(), engine_->GetScissorRect());
    }
    if (isActiveMultiMaterial) {
        multiMaterial->Draw(engine_->GetDrawManager(), engine_->GetViewport(), engine_->GetScissorRect());
    }
    if (isActiveSuzanne) {
        suzanne->Draw(engine_->GetDrawManager(), engine_->GetViewport(), engine_->GetScissorRect());
    }
    if (isActiveFence_) {
        fence_->Draw(engine_->GetDrawManager(), engine_->GetViewport(), engine_->GetScissorRect());
    }
    
    engine_->SetBlend(BlendMode::kBlendModeAdd);
    engine_->SetDepthWrite(PSOManager::DepthWrite::Disable);
    engine_->ApplyParticlePSO();

    if (isActiveParticle) {
        engine_->GetDrawManager()->DrawParticle(engine_->GetViewport(), engine_->GetScissorRect(), particle.get());
    }

    // 2D
    
    engine_->SetBlend(BlendMode::kBlendModeNormal);
    engine_->SetDepthWrite(PSOManager::DepthWrite::Enable);
    engine_->ApplyPSO();

    if (isActiveSprite) {
        engine_->GetDrawManager()->DrawSprite(engine_->GetViewport(), engine_->GetScissorRect(), sprite.get());
    }
}
