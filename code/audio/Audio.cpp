#include "Audio.h"

/// stl
#include <cassert>
#include <iostream>
/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// asset
#include "asset/AssetSystem.h"
#include "asset/manager/SoundAssetManager.h"
/// util
#include "EngineConfig.h"
#include "myFileSystem/MyFileSystem.h"

/// externals
#include "logger/Logger.h"
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif

#pragma comment(lib, "xaudio2.lib")

using namespace OriGine;

Microsoft::WRL::ComPtr<IXAudio2> Audio::xAudio2_;
IXAudio2MasteringVoice* Audio::masterVoice_;

#pragma region "Audio"
/// <summary>
/// オーディオエンジンの静的初期化. 全 Audio インスタンスで共有される XAudio2 エンジンと
/// マスタリングボイスをこの中で一度だけ生成する.
/// XAudio2Create でエンジン本体を作ってからでないと CreateMasteringVoice を呼べないため、
/// 必ずこの順序で呼び出す必要がある.
/// </summary>
void Audio::StaticInitialize() {
    LOG_DEBUG("Start Static Initialize Audio");
    HRESULT result;

    //===================================================================
    // XAudio2 エンジンインスタンス 作成
    //===================================================================
    result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create XAudio2 engine: {}", result);
        assert(false);
    }
    //===================================================================
    // MasteringVoice 作成
    // マスタリングボイスは最終的な音声出力（スピーカー等）へのミキシング先であり、
    // 個々の SourceVoice（PlayTrigger/PlayLoop で生成される）はすべてこれを介して出力される
    //===================================================================
    result = xAudio2_->CreateMasteringVoice(&masterVoice_);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create mastering voice: {}", result);
        assert(false);
    }

    LOG_DEBUG("Complete Static Initialize Audio");
}

/// <summary>
/// オーディオエンジンの静的終了処理.
/// マスタリングボイスの破棄、XAudio2 エンジンの解放を行う.
/// </summary>
void Audio::StaticFinalize() {
    masterVoice_->DestroyVoice();
    xAudio2_.Reset();
}

/// <summary>
/// 単発（トリガー）再生を行う.
/// 既存の SourceVoice が残っている場合は、新しいバッファを積む前に必ず
/// Stop（再生停止）→ FlushSourceBuffers（キュー済みバッファの破棄）→ DestroyVoice（ボイス自体の破棄）
/// の3段階を踏む。再生中のまま DestroyVoice だけを呼ぶとバッファがキューに残った状態で
/// ボイスを破棄することになり、不正な状態遷移としてXAudio2側でエラー・不具合の原因になるため。
/// </summary>
void Audio::PlayTrigger() {
    HRESULT result;

    const SoundAsset* soundAsset = GetSoundAsset();
    if (soundAsset == nullptr) {
        LOG_ERROR("Audio has no sound asset to play. (file: {})", fileName_);
        return;
    }

    if (pSourceVoice_) {
        // 再生を停止し、バッファをクリア
        pSourceVoice_->Stop(0);
        pSourceVoice_->FlushSourceBuffers();
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }

    result = xAudio2_->CreateSourceVoice(&pSourceVoice_, &soundAsset->data.wfex);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create source voice: {}", result);
        assert(false);
    }

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.volume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = soundAsset->data.pBuffer.data();
    buffer.AudioBytes     = soundAsset->data.bufferSize;
    buffer.Flags          = XAUDIO2_END_OF_STREAM;

    result = pSourceVoice_->SubmitSourceBuffer(&buffer);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to submit source buffer: {}", result);
        assert(false);
    }

    result = pSourceVoice_->Start();
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to start source voice: {}", result);
        assert(false);
    }
}

/// <summary>
/// ループ再生を行う.
/// PlayTrigger と同様に、既存の SourceVoice がある場合は Stop → FlushSourceBuffers → DestroyVoice の
/// 順で確実に片付けてから新しいボイスを作り直す（理由は PlayTrigger のコメントを参照）.
/// バッファに LoopCount = XAUDIO2_LOOP_INFINITE を設定することで無限ループ再生を実現する.
/// </summary>
void Audio::PlayLoop() {
    HRESULT result;

    const SoundAsset* soundAsset = GetSoundAsset();
    if (soundAsset == nullptr) {
        LOG_ERROR("Audio has no sound asset to play. (file: {})", fileName_);
        return;
    }

    if (pSourceVoice_) {
        // 再生を停止し、バッファをクリア
        pSourceVoice_->Stop(0);
        pSourceVoice_->FlushSourceBuffers();
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }

    result = xAudio2_->CreateSourceVoice(&pSourceVoice_, &soundAsset->data.wfex);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create source voice: {}", result);
        assert(false);
    }

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.volume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = soundAsset->data.pBuffer.data();
    buffer.AudioBytes     = soundAsset->data.bufferSize;
    buffer.Flags          = XAUDIO2_END_OF_STREAM;
    buffer.LoopBegin      = 0;
    buffer.LoopLength     = 0;
    buffer.LoopCount      = XAUDIO2_LOOP_INFINITE;

    result = pSourceVoice_->SubmitSourceBuffer(&buffer);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to submit source buffer: {}", result);
        assert(false);
    }

    result = pSourceVoice_->Start();
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to start source voice: {}", result);
        assert(false);
    }
}

/// <summary>
/// 音声の再生を開始する. isLoop_ フラグに応じてループ再生／単発再生を切り替える.
/// </summary>
void Audio::Play() {
    if (audioClip_.isLoop_) {
        PlayLoop();
    } else {
        PlayTrigger();
    }
}

/// <summary>
/// 再生を一時停止する.
/// pSourceVoice_ が nullptr の場合はまだ一度も Play されていない状態であり、
/// この関数はその場合を考慮していない（呼び出し側が再生中であることを保証する前提）.
/// </summary>
void Audio::Pause() {
    pSourceVoice_->Stop(0);
}

/// <summary>
/// 音声データを読み込む.
/// 実際のファイル読み込みとキャッシュは SoundAssetManager が担当するため、
/// ここでは参照するアセットの差し替え（旧アセットの解放 → 新アセットの取得）だけを行う.
/// </summary>
/// <param name="_fileName">読み込む WAVE ファイルのパス</param>
void Audio::Load(const std::string& _fileName) {
    // 同じファイルを既に参照しているなら参照カウントを増減させる必要はない
    if (fileName_ == _fileName && audioClip_.soundAssetIndex_ != kInvalidAssetIndex) {
        return;
    }

    fileName_ = _fileName;
    ReloadSoundAsset();
}

/// <summary>
/// 現在の fileName_ に従って音声アセットを取得し直す.
/// 旧アセットを解放してから読み込むため、参照カウントの辻褄が合う.
/// </summary>
void Audio::ReloadSoundAsset() {
    ReleaseSoundAsset();

    if (fileName_.empty()) {
        return;
    }

    auto* manager = AssetSystem::GetInstance()->GetManager<SoundAsset>();
    if (manager == nullptr) {
        LOG_ERROR("SoundAssetManager is not registered.");
        return;
    }
    audioClip_.soundAssetIndex_ = manager->LoadAsset(fileName_);
}

/// <summary>
/// 参照中の音声アセットを解放し、参照を無効化する.
/// </summary>
void Audio::ReleaseSoundAsset() {
    if (audioClip_.soundAssetIndex_ == kInvalidAssetIndex) {
        return;
    }
    if (auto* manager = AssetSystem::GetInstance()->GetManager<SoundAsset>()) {
        manager->ReleaseAsset(audioClip_.soundAssetIndex_);
    }
    audioClip_.soundAssetIndex_ = kInvalidAssetIndex;
}

/// <summary>
/// 参照中の音声アセットを取得する.
/// </summary>
/// <returns>未読み込み・解放済みの場合は nullptr</returns>
const SoundAsset* Audio::GetSoundAsset() const {
    if (audioClip_.soundAssetIndex_ == kInvalidAssetIndex) {
        return nullptr;
    }
    auto* manager = AssetSystem::GetInstance()->GetManager<SoundAsset>();
    if (manager == nullptr) {
        return nullptr;
    }
    return manager->IsAlive(audioClip_.soundAssetIndex_)
               ? &manager->GetAsset(audioClip_.soundAssetIndex_)
               : nullptr;
}

/// <summary>
/// 現在再生中かどうかを判定する.
/// </summary>
/// <returns>再生中なら true, 停止中またはソースボイス未生成なら false</returns>
bool Audio::isPlaying() const {
    // pSourceVoice_ は PlayTrigger/PlayLoop が呼ばれるまで生成されないため、
    // 一度も再生されていない状態や Finalize 後は nullptr になりうる。
    // ここでチェックしないと直後の GetState でヌルポインタ参照になってしまう
    if (pSourceVoice_ == nullptr) {
        return false;
    }

    XAUDIO2_VOICE_STATE state;
    pSourceVoice_->GetState(&state);

    // 再生中のバッファが存在する場合は再生中とみなす
    return state.BuffersQueued > 0;
}

/// <summary>
/// コンポーネントの初期化を行う. 設定されたファイル名から音声データを読み出す.
/// </summary>
/// <param name="_scene">所属シーン（未使用）</param>
/// <param name="_entity">所有者エンティティ（未使用）</param>
void Audio::Initialize(Scene* /*_scene*/, const EntityHandle& /*_entity*/) {
    // ファイル名が設定されていれば音声データを読み込む。
    // デシリアライズ直後は soundAssetIndex_ が未設定なので、Load の
    // 「同じファイルなら何もしない」早期 return には引っかからない
    if (!fileName_.empty()) {
        Load(fileName_);
    }
};

/// <summary>
/// エディタ用 UI 編集処理. デバッグビルドでのみ有効.
/// </summary>
/// <param name="_scene">所属シーン（未使用）</param>
/// <param name="_entity">所有者エンティティ（未使用）</param>
/// <param name="_parentLabel">ImGui のウィジェット ID 重複を避けるための親ラベル</param>
void Audio::Edit(Scene* /*_scene*/, const EntityHandle& /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    std::string label = "LoadFile##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory;
        std::string fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"wav"})) {
            const std::string filePath = kApplicationResourceDirectory + "/" + directory + "/" + fileName;

            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&fileName_, filePath));
            // アセットの差し替えはコマンド確定後に行う。
            // こうしておくと Undo / Redo で fileName_ が巻き戻ったときも、
            // 同じ後処理が走って参照するアセットが追従する
            commandCombo->SetFuncOnAfterCommand([this]() { ReloadSoundAsset(); }, true);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    ImGui::Text("File:%s", fileName_.c_str());

    CheckBoxCommand("Loop  ##" + _parentLabel, audioClip_.isLoop_);
    SlideGuiCommand("Volume##" + _parentLabel, audioClip_.volume_, Config::Audio::kMinVolume, Config::Audio::kMaxVolume);

    label = "Play##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        Play();
    }
    label = "Pause##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        Pause();
    }
#endif // _DEBUG
}

/// <summary>
/// 終了処理を行う. ソースボイスの破棄と音声データのアンロードを行う.
/// pSourceVoice_ が存在する場合のみ Stop → FlushSourceBuffers → DestroyVoice を行う
/// （PlayTrigger 等と同じ理由で、再生中のまま破棄しないようにするため）.
/// </summary>
void Audio::Finalize() {
    if (pSourceVoice_) {
        // 再生を停止し、バッファをクリア
        pSourceVoice_->Stop(0);
        pSourceVoice_->FlushSourceBuffers();
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }
    ReleaseSoundAsset();
}

/// <summary> Audio コンポーネントを JSON にシリアライズする. </summary>
void OriGine::to_json(nlohmann::json& _j, const Audio& _comp) {
    _j["fileName"] = _comp.fileName_;
    _j["isLoop"]   = _comp.audioClip_.isLoop_;
    _j["volume"]   = _comp.audioClip_.volume_;
}

/// <summary> JSON から Audio コンポーネントの設定値を復元する. </summary>
void OriGine::from_json(const nlohmann::json& _j, Audio& _comp) {
    _j.at("fileName").get_to(_comp.fileName_);
    _j.at("isLoop").get_to(_comp.audioClip_.isLoop_);
    _j.at("volume").get_to(_comp.audioClip_.volume_);
}

#pragma endregion "Audio"

/// <summary> コンストラクタ. Initialize カテゴリのシステムとして登録される. </summary>
AudioInitializeSystem::AudioInitializeSystem() : ISystem(SystemCategory::Initialize) {};

/// <summary> デストラクタ. </summary>
AudioInitializeSystem::~AudioInitializeSystem() {}

/// <summary> システムの初期化を行う（このシステムは特に初期化処理を持たない）. </summary>
void AudioInitializeSystem::Initialize() {}
/// <summary> システムの終了処理を行う（このシステムは特に終了処理を持たない）. </summary>
void AudioInitializeSystem::Finalize() {}

/// <summary>
/// エンティティごとの更新処理.
/// エンティティに紐付くすべての Audio コンポーネントを再生する.
/// </summary>
/// <param name="_entity">更新対象のエンティティハンドル</param>
void AudioInitializeSystem::UpdateEntity(const EntityHandle& _entity) {
    // entityの持つ AuidoComponentをすべて取得.
    // 存在していればすべて再生
    auto& components = GetComponents<Audio>(_entity);

    if (components.empty()) {
        return;
    }
    for (auto& audio : components) {
        audio.Play();
    }
}
