#include "Camera.h"

/*開発のUIを出そう*/

#include "../externals/imgui/imgui.h"

#include <cmath>
#include <string>

#include "../function/Math.h"

//コンストラクタ
Camera::Camera() {}

//デストラクタ
Camera::~Camera() {}

//初期化
void Camera::Initialize(int window_width,int window_height) {
    width_ = static_cast<float>(window_width);
    height_ = static_cast<float>(window_height);
    MakeWorldMatrix();
    MakeViewMatrix();
    UpdatePerspectiveFovMatrix();
    UpdateOrthographicMatrix();
    UpdateViewportMatrix();
}

//更新
void Camera::Update(const char *cameraName) {

#ifdef _DEBUG

    std::string name = std::string("Camera: ") + cameraName;

    //ImGui

    //カメラウィンドウを作り出す
    ImGui::Begin(name.c_str());
    // translate
    ImGui::DragFloat3("translate", &translate_.x, 0.1f);
    // rotate
    ImGui::DragFloat3("rotate", &rotate_.x, 0.1f);
    //入力終了
    ImGui::End();

    //値に応じて行列も更新
    UpdateMatrix();

#endif // _DEBUG

}

//ワールド行列の作成
void Camera::MakeWorldMatrix() {

    worldMatrix_ = Math::MakeAffineMatrix(scale_, rotate_, translate_);

}

//ビュー行列の作成
void Camera::MakeViewMatrix() {

    viewMatrix_ =Math::Inverse(worldMatrix_);

}

//透視投影行列の更新
void Camera::UpdatePerspectiveFovMatrix() {

    perspectiveFovMatrix_ = Math::MakePerspectiveFovMatrix(fovAngleY_, aspectRatio_, nearZ_, farZ_);

}

//正射行列の更新
void Camera::UpdateOrthographicMatrix() {

    orthographicMatrix_ = Math::MakeOrthographicMatrix(left_, top_, right_, bottom_, nearClip_, farClip_);

}

//ビューポート行列の更新
void Camera::UpdateViewportMatrix() {
  
    viewportMatrix_ = Math::MakeViewportMatrix(leftTop_.x, leftTop_.y, width_, height_, minDepth_, maxDepth_);

}

//各行列の更新
void Camera::UpdateMatrix() {
    MakeWorldMatrix();
    MakeViewMatrix();
    UpdatePerspectiveFovMatrix();
    UpdateOrthographicMatrix();
    UpdateViewportMatrix();
}

// カメラ行列を取得する
Matrix4x4 Camera::GetCameraMatrix() { return Math::MakeAffineMatrix(scale_, rotate_, translate_); }