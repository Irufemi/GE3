#pragma once

#include "../manager/AudioManager.h"
#include <string>

class Bgm {
private:

    IXAudio2SourceVoice* voice_ = nullptr;

    float volume_ = 0.01f;

    int selectedCat_ = 0;

    int selectedTrack_ = 0;

    AudioManager* audioManager_ = nullptr;

public:
    Bgm() = default;
    ~Bgm();

    // 初期化
    void Initialize(AudioManager* audioManager);

    // 更新
    void Update();

    void Play(const std::string& category, const std::string& track, bool loop = true);
    void PlayFirstTrack(); // 最初のカテゴリと最初のトラックで再生
    void Stop();

    void SetVolume(float volume);
    float GetVolume() const { return volume_; }

    int GetSelectedCategoryIndex() const { return selectedCat_; }
    int GetSelectedTrackIndex() const { return selectedTrack_; }
    void SetSelectedCategoryIndex(int index) { selectedCat_ = index; }
    void SetSelectedTrackIndex(int index) { selectedTrack_ = index; }

};