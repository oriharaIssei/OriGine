#pragma once

/// microsoft
#include <wrl.h>
/// api
#include <xaudio2.h>
/// stl
#include <memory>
#include <stdint.h>
#include <string>
/// engine
#include "component/IComponent.h"
#include "EngineConfig.h"
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// チャンクヘッダ
/// </summary>
struct ChunkHeader {
    char id[4];
    int32_t size;
};
/// <summary>
/// RIFFヘッダ
/// </summary>
struct RiffHeader {
    ChunkHeader chunk;
    char type[4];
};
/// <summary>
/// fmtチャンク
/// </summary>
struct FormatChunk {
    ChunkHeader chunk;
    WAVEFORMATEX fmt;
};
/// <summary>
/// 音声データ
/// </summary>
struct SoundData {
    /// <summary>波形フォーマット</summary>
    WAVEFORMATEX wfex;
    /// <summary>バッファの先頭ポインタ</summary>
    BYTE* pBuffer = nullptr;
    /// <summary>バッファサイズ</summary>
    uint32_t bufferSize;
};

/// <summary>
/// オーディオクリップ情報. 音声データと再生設定を保持する.
/// </summary>
class AudioClip {
public:
    /// <summary>音声データ</summary>
    SoundData data_;
    /// <summary>ループ再生するか</summary>
    bool isLoop_ = false;
    /// <summary>音量 (0.0 ～ 2.0)</summary>
    float volume_ = Config::Audio::kDefaultVolume;
};

/// <summary>
/// 音声を再生するためのコンポーネント.
/// XAudio2 を使用して WAVE ファイルの再生を管理する.
/// </summary>
class Audio
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const Audio& _comp);
    friend void from_json(const nlohmann::json& _j, Audio& _comp);

public:
    /// <summary>
    /// オーディオエンジンの静的初期化を行う.
    /// XAudio2 インスタンスとマスターボイスを作成する.
    /// </summary>
    static void StaticInitialize();

    /// <summary>
    /// オーディオエンジンの静的終了処理を行う.
    /// </summary>
    static void StaticFinalize();

    Audio() {}
    ~Audio() {}

    /// <summary>
    /// コンポーネントの初期化を行う.
    /// 設定されたファイル名から音声データを読み出す.
    /// </summary>
    /// <param name="_scene">所属シーン（未使用）</param>
    /// <param name="_entity">所有者エンティティ（未使用）</param>
    void Initialize(Scene* _scene, EntityHandle _entity) override;

    /// <summary>
    /// エディタ用 UI 編集処理.
    /// </summary>
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    /// <summary>
    /// 終了処理を行う. ソースボイスの破棄と音声データのアンロードを行う.
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// 音声の再生を開始する.
    /// </summary>
    void Play();

    /// <summary>
    /// 再生を一時停止する.
    /// </summary>
    void Pause();

private:
    /// <summary>
    /// 単発（トリガー）再生を行う.
    /// </summary>
    void PlayTrigger();

    /// <summary>
    /// ループ再生を行う.
    /// </summary>
    void PlayLoop();

    /// <summary>
    /// 指定されたパスの WAVE ファイルをロードする.
    /// </summary>
    /// <param name="_fileName">ファイルパス</param>
    /// <returns>読み込まれた音声データ</returns>
    SoundData LoadWave(const std::string& _fileName);

    /// <summary>
    /// 音声データをメモリから解放する.
    /// </summary>
    void SoundUnLoad();

private:
    /// <summary>XAudio2 エンジン</summary>
    static Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    /// <summary>マスターボイス（すべての音の出力先）</summary>
    static IXAudio2MasteringVoice* masterVoice_;

    /// <summary>読み込み対象のファイル名</summary>
    std::string fileName_;

    /// <summary>オーディオクリップ</summary>
    AudioClip audioClip_;
    /// <summary>再生用ソースボイス</summary>
    IXAudio2SourceVoice* pSourceVoice_ = nullptr;

public:
    /// <summary>
    /// 音声データを読み込む.
    /// </summary>
    /// <param name="_fileName">ファイル名</param>
    void Load(const std::string& _fileName);

    /// <summary>
    /// 現在再生中かどうかを判定する.
    /// </summary>
    /// <returns>再生中なら true, 停止中またはバッファが空なら false</returns>
    bool isPlaying() const;
};

/// <summary>
/// 初期化時に Audio コンポーネントの再生を開始するためのシステム.
/// </summary>
class AudioInitializeSystem
    : public ISystem {
public:
    AudioInitializeSystem();
    ~AudioInitializeSystem() override;

    /// <summary>
    /// システムの初期化を行う.
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// システムの終了処理を行う.
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// エンティティごとの更新処理.
    /// エンティティに紐付くすべての Audio コンポーネントを再生する.
    /// </summary>
    /// <param name="_entity">更新対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _entity) override;
};

} // namespace OriGine
