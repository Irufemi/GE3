#include "Bgm.h"

#include <algorithm>

#include "../externals/imgui/imgui.h" 

Bgm::~Bgm() {
    Stop();
}

void Bgm::Initialize(AudioManager* audioManager){
    this->audioManager_ = audioManager;
}

void Bgm::Play(const std::string& category, const std::string& track, bool loop) {
    Stop();
    std::string key = category + "/" + track;
    voice_ = audioManager_->Play(audioManager_->GetSoundData(key), loop, volume_);
}

void Bgm::PlayFirstTrack() {
    auto cats = audioManager_->GetCategories();
    if (!cats.empty()) {
        selectedCat_ = 0;
        auto tracks = audioManager_->GetSoundNames(cats[selectedCat_]);
        if (!tracks.empty()) {
            selectedTrack_ = 0;
            Play(cats[selectedCat_], tracks[selectedTrack_], true);
        }
    }
}

void Bgm::Stop() {
    if (voice_) {
        audioManager_->Stop(voice_);
        voice_ = nullptr;
    }
}

void Bgm::SetVolume(float volume) {
    volume_ = volume;
    if (voice_) {
        voice_->SetVolume(volume_);
    }
}

void Bgm::Update() {

    ImGui::Begin("Audio Settings");

    auto cats = audioManager_->GetCategories();
    if (!cats.empty()) {
        selectedCat_ = std::clamp(selectedCat_, 0, (int)cats.size() - 1);
        if (ImGui::BeginCombo("Category", cats[selectedCat_].c_str())) {
            for (int i = 0; i < cats.size(); ++i) {
                bool sel = (i == selectedCat_);
                if (ImGui::Selectable(cats[i].c_str(), sel)) {
                    selectedCat_ = i;
                    selectedTrack_ = 0;
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        auto tracks = audioManager_->GetSoundNames(cats[selectedCat_]);
        if (!tracks.empty()) {
            selectedTrack_ = std::clamp(selectedTrack_, 0, (int)tracks.size() - 1);
            if (ImGui::BeginCombo("BGM Track", tracks[selectedTrack_].c_str())) {
                for (int i = 0; i < tracks.size(); ++i) {
                    bool sel = (i == selectedTrack_);
                    if (ImGui::Selectable(tracks[i].c_str(), sel)) {
                        selectedTrack_ = i;
                        Play(cats[selectedCat_], tracks[i], true);
                    }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
    }

    float tempVolume = volume_;
    if (ImGui::SliderFloat("Volume", &tempVolume, 0.0f, 1.0f)) {
        SetVolume(tempVolume);
    }

    ImGui::End();
}