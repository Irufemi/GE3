#include "InputManager.h"
#include <cmath>

void InputManager::Initialize() {
    currentKeys_.fill(0);
    previousKeys_.fill(0);
    ZeroMemory(&state_, sizeof(XINPUT_STATE));
    ZeroMemory(&prevState_, sizeof(XINPUT_STATE));
}

void InputManager::Update() {
    // 前回の状態を保存
    previousKeys_ = currentKeys_;
    prevState_ = state_;

    // --- キーボード入力をWin32 APIで取得 ---
    GetKeyboardState(currentKeys_.data());

    // --- Xboxコントローラー入力を取得 ---
    ZeroMemory(&state_, sizeof(XINPUT_STATE));
    DWORD result = XInputGetState(0, &state_);
    if (result != ERROR_SUCCESS) {
        ZeroMemory(&state_, sizeof(XINPUT_STATE)); // 接続なし
    }
}

#pragma region キーボード処理
bool InputManager::IsKeyDown(uint8_t key) const {
    return (currentKeys_[key] & 0x80) != 0;
}

bool InputManager::IsKeyUp(uint8_t key) const {
    return !(currentKeys_[key] & 0x80);
}

bool InputManager::IsKeyPressed(uint8_t key) const {
    return !(previousKeys_[key] & 0x80) && (currentKeys_[key] & 0x80);
}

bool InputManager::IsKeyReleased(uint8_t key) const {
    return (previousKeys_[key] & 0x80) && !(currentKeys_[key] & 0x80);
}
#pragma endregion

#pragma region コントローラー処理
bool InputManager::IsButtonDown(WORD button) const {
    return (state_.Gamepad.wButtons & button) != 0;
}

bool InputManager::IsButtonUp(WORD button) const {
    return !(state_.Gamepad.wButtons & button);
}

bool InputManager::IsButtonPressed(WORD button) const {
    return !(prevState_.Gamepad.wButtons & button) && (state_.Gamepad.wButtons & button);
}

bool InputManager::IsButtonReleased(WORD button) const {
    return (prevState_.Gamepad.wButtons & button) && !(state_.Gamepad.wButtons & button);
}

float InputManager::GetLeftStickX() const {
    float norm = (float)state_.Gamepad.sThumbLX / 32767.0f;
    return (std::fabs(norm) < deadZoneLeft_) ? 0.0f : norm;
}

float InputManager::GetLeftStickY() const {
    float norm = (float)state_.Gamepad.sThumbLY / 32767.0f;
    return (std::fabs(norm) < deadZoneLeft_) ? 0.0f : norm;
}

float InputManager::GetRightStickX() const {
    float norm = (float)state_.Gamepad.sThumbRX / 32767.0f;
    return (std::fabs(norm) < deadZoneRight_) ? 0.0f : norm;
}

float InputManager::GetRightStickY() const {
    float norm = (float)state_.Gamepad.sThumbRY / 32767.0f;
    return (std::fabs(norm) < deadZoneRight_) ? 0.0f : norm;
}

float InputManager::GetLeftTrigger() const {
    return state_.Gamepad.bLeftTrigger / 255.0f;
}

float InputManager::GetRightTrigger() const {
    return state_.Gamepad.bRightTrigger / 255.0f;
}
#pragma endregion