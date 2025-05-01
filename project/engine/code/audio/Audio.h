#pragma once

/// api
#include <xaudio2.h>

#include <wrl.h>
/// stl
#include <memory>
#include <stdint.h>
#include <string>

/// engine
#include "component/IComponent.h"

struct ChunkHeader {
    char id[4];
    int32_t size;
};
struct RiffHeader {
    ChunkHeader chunk;
    char type[4];
};
struct FormatChunk {
    ChunkHeader chunk;
    WAVEFORMATEX fmt;
};

struct SoundData {
    WAVEFORMATEX wfex;
    BYTE* pBuffer = nullptr;
    uint32_t bufferSize;
};

class AudioClip {
public:
    SoundData data_;
    bool isLoop_  = false;
    float valume_ = 0.5f;
    bool isPlay_  = false; // 再生するかどうか
};

class Audio
    : public IComponent {
    friend void to_json(nlohmann::json& j, const Audio& t);
    friend void from_json(const nlohmann::json& j, Audio& t);

public:
    static void StaticInitialize();
    static void StaticFinalize();

    Audio() {}
    ~Audio() {}

    void Initialize(GameEntity* /*_entity*/) override {
        if (!fileName_.empty()) {
            audioClip_.data_ = LoadWave(fileName_);
        }
    };

    bool Edit() override;

    void Finalize() override;

    void Play() {
        if (audioClip_.isLoop_) {
            PlayLoop();
        } else {
            PlayTrigger();
        }
    }

    void Pause();

private:
    void PlayTrigger();
    void PlayLoop();

    SoundData LoadWave(const std::string& fileName);
    void SoundUnLoad();

private:
    static Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    static IXAudio2MasteringVoice* masterVoice_;

    std::string fileName_;

    AudioClip audioClip_;
    IXAudio2SourceVoice* pSourceVoice_ = nullptr;

public:
    void Load(const std::string& fileName) {
        fileName_        = fileName;
        audioClip_.data_ = LoadWave(fileName_);
    }

    bool isPlaying() const {
        if (pSourceVoice_ == nullptr) {
            return false; // 再生用のソースボイスが存在しない場合は再生中ではない
        }

        XAUDIO2_VOICE_STATE state;
        pSourceVoice_->GetState(&state);

        // 再生中のバッファが存在する場合は再生中とみなす
        return state.BuffersQueued > 0;
    }
};
