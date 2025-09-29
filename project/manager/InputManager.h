#pragma once
#include <Windows.h>
#include <Xinput.h>
#include <array>

#pragma comment(lib, "Xinput.lib")

class InputManager {
public:

    static const int KEY_COUNT = 256;

private:

    // キーボード（Win32 API）
    std::array<BYTE, KEY_COUNT> currentKeys_;
    std::array<BYTE, KEY_COUNT> previousKeys_;

    // Xboxコントローラー（XInput）
    XINPUT_STATE state_{};
    XINPUT_STATE prevState_{};

    // スティックのデッドゾーン
    float deadZoneLeft_ = 0.2f;
    float deadZoneRight_ = 0.2f;

public:

    InputManager() = default;
    ~InputManager() = default;

    void Initialize();

    void Update();

    // --- キーボード入力 ---
    bool IsKeyDown(uint8_t key) const;
    bool IsKeyUp(uint8_t key) const;
    bool IsKeyPressed(uint8_t key) const;
    bool IsKeyReleased(uint8_t key) const;

    // --- コントローラー入力 ---
    bool IsButtonDown(WORD button) const;
    bool IsButtonUp(WORD button) const;
    bool IsButtonPressed(WORD button) const;
    bool IsButtonReleased(WORD button) const;

    float GetLeftStickX() const;
    float GetLeftStickY() const;
    float GetRightStickX() const;
    float GetRightStickY() const;

    float GetLeftTrigger() const;
    float GetRightTrigger() const;

};