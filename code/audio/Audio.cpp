#include "Audio.h"

/// stl
#include <cassert>
#include <fstream>
#include <iostream>
/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
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

    if (pSourceVoice_) {
        // 再生を停止し、バッファをクリア
        pSourceVoice_->Stop(0);
        pSourceVoice_->FlushSourceBuffers();
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }

    result = xAudio2_->CreateSourceVoice(&pSourceVoice_, &audioClip_.data_.wfex);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create source voice: {}", result);
        assert(false);
    }

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.volume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = audioClip_.data_.pBuffer.data();
    buffer.AudioBytes     = audioClip_.data_.bufferSize;
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

    if (pSourceVoice_) {
        // 再生を停止し、バッファをクリア
        pSourceVoice_->Stop(0);
        pSourceVoice_->FlushSourceBuffers();
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }

    result = xAudio2_->CreateSourceVoice(&pSourceVoice_, &audioClip_.data_.wfex);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create source voice: {}", result);
        assert(false);
    }

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.volume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = audioClip_.data_.pBuffer.data();
    buffer.AudioBytes     = audioClip_.data_.bufferSize;
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
/// </summary>
/// <param name="_fileName">読み込む WAVE ファイルのパス</param>
void Audio::Load(const std::string& _fileName) {
    fileName_        = _fileName;
    audioClip_.data_ = LoadWave(_fileName);
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
    // ファイル名が設定されていれば音声データを読み込む
    if (!fileName_.empty()) {
        audioClip_.data_ = LoadWave(fileName_);
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
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName_, {"wav"})) {
            OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<std::string>>(&fileName_, kApplicationResourceDirectory + "/" + directory + "/" + fileName_));

            audioClip_.data_ = LoadWave(kApplicationResourceDirectory + "/" + directory + "/" + fileName_);
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
    SoundUnLoad();
}

/// <summary>
/// 指定されたパスの WAVE ファイルをロードする.
/// RIFF/WAVE 形式のチャンク構造を先頭から順に走査し、"fmt "チャンクから波形フォーマット(WAVEFORMATEX)を、
/// "data"チャンクから実際の音声データ本体を取り出す。両方が見つかるまでファイル終端まで走査し、
/// それ以外の未知のチャンク（メタデータ等）は読み飛ばす。
/// </summary>
/// <param name="_fileName">読み込む WAVE ファイルのパス</param>
/// <returns>読み込まれた音声データ（失敗時は空の SoundData）</returns>
SoundData Audio::LoadWave(const std::string& _fileName) {
    std::ifstream file(_fileName, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: {}", _fileName);
        return {};
    }

    RiffHeader riff;
    file.read(reinterpret_cast<char*>(&riff), sizeof(riff));

    if (strncmp(riff.chunk.id, "RIFF", 4) != 0 || strncmp(riff.type, "WAVE", 4) != 0) {
        LOG_ERROR("Invalid RIFF or WAVE header");
        return {};
    }

    FormatChunk format{};
    ChunkHeader chunk;

    bool foundFmt  = false;
    bool foundData = false;
    DWORD dataSize = 0;
    std::vector<BYTE> dataBuffer;

    // RIFF チャンクを先頭から順に走査し、fmt チャンク（フォーマット情報）と data チャンク（音声データ本体）を探し出す
    while (file.read(reinterpret_cast<char*>(&chunk), sizeof(chunk))) {
        std::streampos nextChunk = file.tellg();
        nextChunk += chunk.size; // 次のチャンクの開始位置を算出

        if (strncmp(chunk.id, "fmt ", 4) == 0) {
            foundFmt = true;
            file.read(reinterpret_cast<char*>(&format.fmt), chunk.size); // フォーマット情報を読み込む
        } else if (strncmp(chunk.id, "data", 4) == 0) {
            foundData = true;
            dataBuffer.resize(chunk.size);
            file.read(reinterpret_cast<char*>(dataBuffer.data()), chunk.size); // 音声データ本体を読み込む
            dataSize = chunk.size;
        } else {
            // 未使用のチャンクはスキップ
            file.seekg(chunk.size, std::ios::cur);
        }

        file.seekg(nextChunk); // 次のチャンクの位置へシーク
    }

    if (!foundFmt || !foundData) {
        LOG_ERROR("Required fmt or data chunk not found");
        return {};
    }

    SoundData soundData{};
    soundData.wfex       = format.fmt;
    soundData.pBuffer    = std::move(dataBuffer);
    soundData.bufferSize = dataSize;

    return soundData;
}

/// <summary>
/// 音声データをメモリから解放する.
/// バッファを clear() だけでなく shrink_to_fit() まで行い、確保済みキャパシティも実際に解放する.
/// </summary>
void Audio::SoundUnLoad() {
    audioClip_.data_.pBuffer.clear();
    audioClip_.data_.pBuffer.shrink_to_fit();
    audioClip_.data_.bufferSize = 0;
    audioClip_.data_.wfex       = {};
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
