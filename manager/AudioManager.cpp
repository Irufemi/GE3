#include "AudioManager.h"
#include <cassert>
#include <filesystem> // フォルダ内のファイルを探索するために使用
#include <algorithm>  // 文字列を小文字に変換するために使用

#pragma comment (lib,"xaudio2.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

AudioManager::~AudioManager() {
    Finalize();
}

void AudioManager::Initialize() {
    // XAudio2エンジンの生成
    HRESULT hr = XAudio2Create(&pXAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(hr));

    // マスターボイスの生成
    hr = pXAudio2_->CreateMasteringVoice(&pMasteringVoice_);
    assert(SUCCEEDED(hr));
}

//Media Foundationの初期化
void AudioManager::StartUp() {


    /*サウンド再生*/

    ///Microsoft Media Foundation

    //Media Foundationの初期化
    MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
}

void AudioManager::Finalize() {
    StopAll(); // すべてのVoiceを安全に停止＆Destroy

    if (pMasteringVoice_) {
        pMasteringVoice_->DestroyVoice();
        pMasteringVoice_ = nullptr;
    }
    if (pXAudio2_) {
        pXAudio2_->Release();
        pXAudio2_ = nullptr;
    }
    MFShutdown();
}

void AudioManager::LoadAllSoundsFromFolder(const std::string& folderPath) {
    soundRegistry_.clear();
    categoryMap_.clear();
    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_directory()) {
            std::string category = entry.path().filename().string();
            LoadSoundsFromFolder(entry.path().string(), category);
        }
    }
}

// サブフォルダ単位でロードするオーバーロード版
void AudioManager::LoadSoundsFromFolder(const std::string& folderPath, const std::string& category) {
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext != ".wav" && ext != ".mp3" && ext != ".wma") continue;

        std::string filename = entry.path().filename().string();
        std::wstring wpath = entry.path().wstring();

        // キーは "カテゴリ/ファイル名" にする
        std::string key = category + "/" + filename;
        if (soundRegistry_.count(key) == 0) {
            auto sd = std::make_shared<Sound>();
            if (sd->Load(wpath)) {
                soundRegistry_[key] = sd;
                categoryMap_[category].push_back(filename);

            }

        }

    }
    // カテゴリごとにソート
    auto& names = categoryMap_[category];
    std::sort(names.begin(), names.end());

}

std::vector<std::string> AudioManager::GetSoundNames(const std::string& category) const {
    auto it = categoryMap_.find(category);
    if (it == categoryMap_.end()) return {};
    return it->second;
}

std::shared_ptr<Sound> AudioManager::GetSoundData(const std::string& name) const {
    auto it = soundRegistry_.find(name);
    if (it != soundRegistry_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> AudioManager::GetCategories() const {
    std::vector<std::string> cats;
    cats.reserve(categoryMap_.size());
    for (auto const& [cat, _] : categoryMap_) {
        cats.push_back(cat);

    }
    std::sort(cats.begin(), cats.end());
    return cats;
}

IXAudio2SourceVoice* AudioManager::Play(std::shared_ptr<Sound> soundData, bool loop, float volume) {
    if (!pXAudio2_ || !soundData) {
        return nullptr;
    }

    IXAudio2SourceVoice* pSourceVoice{ nullptr };
    HRESULT hr = pXAudio2_->CreateSourceVoice(&pSourceVoice, soundData->GetFormat());
    if (FAILED(hr) || !pSourceVoice) {
        return nullptr;
    }

    // 音量を設定
    pSourceVoice->SetVolume(volume);

    // 再生するオーディオバッファの準備
    XAUDIO2_BUFFER buffer{ 0 };
    buffer.pAudioData = soundData->GetData();
    buffer.Flags = XAUDIO2_END_OF_STREAM; // これで再生が最後まで行くとストリームが終了したとみなされる
    buffer.AudioBytes = soundData->GetSize();
    if (loop) {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE; // ループ再生
    }

    // バッファをソースボイスに送信
    hr = pSourceVoice->SubmitSourceBuffer(&buffer);
    assert(SUCCEEDED(hr));

    // 再生開始
    hr = pSourceVoice->Start(0);
    assert(SUCCEEDED(hr));

    // 生成したソースボイスのポインタを返す（外部で音量変更などに使うため）
    // 管理リストに追加してから返す
    activeVoices_.push_back(pSourceVoice);
    return pSourceVoice;
}

void AudioManager::Stop(IXAudio2SourceVoice*& voice) {
    if (voice) {
        voice->Stop(0);
        voice->DestroyVoice();
        // 管理リストから安全に除去
        auto it = std::remove(activeVoices_.begin(), activeVoices_.end(), voice);
        if (it != activeVoices_.end()) {
            activeVoices_.erase(it, activeVoices_.end());
        }
        voice = nullptr;  // 呼び出し側ポインタもクリア
    }
}

void AudioManager::StopAll() {
    // すべてのSourceVoiceを安全に停止・Destroy
    for (auto& voice : activeVoices_) {
        if (voice) {
            voice->Stop(0);
            voice->DestroyVoice();
            voice = nullptr;
        }
    }
    activeVoices_.clear();
}