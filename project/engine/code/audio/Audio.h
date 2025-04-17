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
    BYTE* pBuffer;
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
public:
    static void StaticInitialize();
    static void StaticFinalize();

    Audio() {}
    ~Audio() {}

    void Initialize(GameEntity* /*_entity*/) override {};

    void Save(BinaryWriter& _writer) override {
        _writer.Write("fileName", fileName_);
        _writer.Write("isLoop", audioClip_.isLoop_);
        _writer.Write("valume", audioClip_.valume_);
        _writer.Write("isPlay", audioClip_.isPlay_);
    };
    void Load(BinaryReader& _reader) override {
        _reader.Read("fileName", fileName_);
        if (fileName_.empty()) {
            audioClip_.data_ = LoadWave(fileName_);
        }

        _reader.Read("isLoop", audioClip_.isLoop_);
        _reader.Read("valume", audioClip_.valume_);
        _reader.Read("isPlay", audioClip_.isPlay_);
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
