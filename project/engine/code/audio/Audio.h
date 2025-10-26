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
#include "system/ISystem.h"


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
    float volume_ = 0.5f;
};

/// <summary>
/// 音を再生するためのコンポーネント
/// </summary>
class Audio
    : public IComponent {
    friend void to_json(nlohmann::json& j, const Audio& t);
    friend void from_json(const nlohmann::json& j, Audio& t);

public:
    static void StaticInitialize();
    static void StaticFinalize();

    Audio() {}
    ~Audio() {}

    void Initialize(Entity* /*_entity*/) override;

    void Edit(Scene* _scene,Entity* _entity,[[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override;

    /// <summary>
    /// 再生を開始します。
    /// </summary>
    void Play();
    /// <summary>
    /// 再生を一時停止します。
    /// </summary>
    void Pause();

private:
    /// <summary>
    /// 一度だけ再生します。
    /// </summary>
    void PlayTrigger();
    /// <summary>
    /// ループ再生します。
    /// </summary>
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
    /// <summary>
    /// 音声データを読み込みます。
    /// </summary>
    /// <param name="fileName"></param>
    void Load(const std::string& fileName);

    /// <summary>
    /// 現在再生中かどうかを取得します。
    /// </summary>
    /// <returns></returns>
    bool isPlaying() const;
};

/// <summary>
/// Audioをシーン初期化時に再生するシステム
/// </summary>
class AudioInitializeSystem
    : public ISystem {
public:
    AudioInitializeSystem();
    ~AudioInitializeSystem() override;

    void Initialize() override;
    void Finalize() override;

    void UpdateEntity(Entity* entity) override;
};
