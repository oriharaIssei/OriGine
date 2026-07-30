#pragma once

/// microsoft
#include <wrl.h>
/// api
#include <xaudio2.h>
/// stl
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

/// engine
// asset
#include "asset/manager/AssetManager.h"
#include "asset/SoundAsset.h"
// ecs
#include "component/IComponent.h"
#include "EngineConfig.h"
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// オーディオクリップ情報. 参照する音声アセットと再生設定を保持する.
/// 波形データそのものは SoundAssetManager が一元管理しており、ここでは
/// そのインデックスだけを持つ（同じ WAVE を複数コンポーネントが使っても実体は共有される）.
/// </summary>
class AudioClip {
public:
    /// <summary>参照している音声アセットのインデックス</summary>
    size_t soundAssetIndex_ = kInvalidAssetIndex;
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
    void Initialize(Scene* _scene, const EntityHandle& _entity) override;

    /// <summary>
    /// エディタ用 UI 編集処理.
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;

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
    /// 参照中の音声アセットを解放し、参照を無効化する.
    /// </summary>
    void ReleaseSoundAsset();

    /// <summary>
    /// 現在の fileName_ に従って音声アセットを取得し直す.
    /// </summary>
    void ReloadSoundAsset();

    /// <summary>
    /// 参照中の音声アセットを取得する.
    /// </summary>
    /// <returns>未読み込み・解放済みの場合は nullptr</returns>
    const SoundAsset* GetSoundAsset() const;

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
    /// 音声データを読み込む. 実体の読み込み・キャッシュは SoundAssetManager が行う.
    /// </summary>
    /// <param name="_fileName">ファイル名</param>
    void Load(const std::string& _fileName);

    /// <summary>参照している音声アセットのインデックスを取得する.</summary>
    size_t GetSoundAssetIndex() const { return audioClip_.soundAssetIndex_; }

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
    void UpdateEntity(const EntityHandle& _entity) override;
};

} // namespace OriGine
