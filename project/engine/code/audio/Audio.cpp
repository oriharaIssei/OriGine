#include "Audio.h"

/// stl
#include <cassert>
#include <fstream>
#include <iostream>

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// lib
#include "logger/Logger.h"
#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include "imgui/imgui.h"
#endif

#pragma comment(lib, "xaudio2.lib")

Microsoft::WRL::ComPtr<IXAudio2> Audio::xAudio2_;
IXAudio2MasteringVoice* Audio::masterVoice_;

void Audio::StaticInitialize() {
    HRESULT result;

    //===================================================================
    // XAudio2 エンジンインスタンス 作成
    //===================================================================
    result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(result));
    //===================================================================
    // MasteringVoice 作成
    //===================================================================
    result = xAudio2_->CreateMasteringVoice(&masterVoice_);
    assert(SUCCEEDED(result));
}

void Audio::StaticFinalize() {
    masterVoice_->DestroyVoice();
    xAudio2_.Reset();
}

void Audio::PlayTrigger() {

    HRESULT result;

    if (pSourceVoice_) {
        pSourceVoice_->DestroyVoice(); // 修正箇所
        pSourceVoice_ = nullptr;
    }

    pSourceVoice_ = nullptr;
    result        = xAudio2_->CreateSourceVoice(&pSourceVoice_, &audioClip_.data_.wfex);
    assert(SUCCEEDED(result));

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.valume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = audioClip_.data_.pBuffer;
    buffer.AudioBytes     = audioClip_.data_.bufferSize;
    buffer.Flags          = XAUDIO2_END_OF_STREAM;

    result = pSourceVoice_->SubmitSourceBuffer(&buffer);
    result = pSourceVoice_->Start();
}

void Audio::PlayLoop() {
    HRESULT result;

    if (pSourceVoice_) {
        pSourceVoice_->DestroyVoice(); // 修正箇所
        pSourceVoice_ = nullptr;
    }

    pSourceVoice_ = nullptr;
    result        = xAudio2_->CreateSourceVoice(&pSourceVoice_, &audioClip_.data_.wfex);
    assert(SUCCEEDED(result));

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.valume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = audioClip_.data_.pBuffer;
    buffer.AudioBytes     = audioClip_.data_.bufferSize;
    buffer.Flags          = XAUDIO2_END_OF_STREAM; // バッファの終端を示す
    buffer.LoopBegin      = 0; // ループの開始位置（先頭から）
    buffer.LoopLength     = 0; // ループする範囲（全体）
    buffer.LoopCount      = XAUDIO2_LOOP_INFINITE; // 無限ループ

    result = pSourceVoice_->SubmitSourceBuffer(&buffer);
    result = pSourceVoice_->Start();

    assert(SUCCEEDED(result));
}

void Audio::Pause() {
    pSourceVoice_->Stop(0);
}

bool Audio::Edit() {
#ifdef _DEBUG
    bool isEdit = false;
    if (ImGui::Button("LoadFile")) {
        std::string directory;
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName_, {"wav"})) {
            fileName_        = kApplicationResourceDirectory + "/" + directory + "/" + fileName_;
            audioClip_.data_ = LoadWave(fileName_);

            if (audioClip_.data_.pBuffer) {
                isEdit = true;
            }
        }
    }

    ImGui::Text("File:%s", fileName_.c_str());
    isEdit |= ImGui::Checkbox("Loop", &audioClip_.isLoop_);
    isEdit |= ImGui::SliderFloat("Volume", &audioClip_.valume_, 0.0f, 2.0f);
    isEdit |= ImGui::Checkbox("Play", &audioClip_.isPlay_);

    if (ImGui::Button("Test Play")) {
        Play();
    }

    return isEdit;
#else
    return false;
#endif // _DEBUG
}

void Audio::Finalize() {
    SoundUnLoad();
}

SoundData Audio::LoadWave(const std::string& fileName) {
    //===================================================================
    // file Open
    //===================================================================
    std::ifstream file;
    file.open(fileName, std::ios_base::binary);
    assert(file.is_open());

    //===================================================================
    // Load file
    //===================================================================
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));

    // ファイルが RIFF か チェック
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
        assert(false);
    }
    // フォーマットが WAVE か チェック
    if (strncmp(riff.type, "WAVE", 4) != 0) {
        assert(false);
    }

    FormatChunk format = {};
    file.read((char*)&format, sizeof(ChunkHeader));
    // チャンクヘッダーの確認
    if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
        assert(false);
    }
    // チャンク本体の 確認
    assert(format.chunk.size <= sizeof(format.fmt));

    file.read((char*)&format.fmt, format.chunk.size);
    ChunkHeader data{};
    file.read((char*)&data, sizeof(data));
    if (strncmp(data.id, "JUNK", 4) == 0) {
        // 読み取り位置を JUNKチャンク 終了位置まで 進める
        file.seekg(data.size, std::ios_base::cur);
        // 再読み込み
        file.read((char*)&data, sizeof(data));
    }

    if (strncmp(data.id, "data", 4) != 0) {
        assert(false);
    }

    char* pBuff = nullptr;
    if (data.size > 0) {
        pBuff = new char[data.size];
        file.read(pBuff, data.size);
    } else {
        assert(false);
    }

    file.close();

    SoundData soundData  = {};
    soundData.wfex       = format.fmt;
    soundData.pBuffer    = reinterpret_cast<BYTE*>(pBuff);
    soundData.bufferSize = data.size;

    return soundData;
}

void Audio::SoundUnLoad() {
    if (audioClip_.data_.pBuffer) {
        std::cout << "pBuffer address: " << static_cast<void*>(audioClip_.data_.pBuffer) << std::endl;
        delete[] audioClip_.data_.pBuffer;
        audioClip_.data_.pBuffer = nullptr;
    }
    audioClip_.data_.bufferSize = 0;
    audioClip_.data_.wfex       = {};
}

void to_json(nlohmann::json& j, const Audio& t) {
    j["fileName"] = t.fileName_;
    j["isLoop"]   = t.audioClip_.isLoop_;
    j["valume"]   = t.audioClip_.valume_;
    j["isPlay"]   = t.audioClip_.isPlay_;
}

void from_json(const nlohmann::json& j, Audio& t) {
    j.at("fileName").get_to(t.fileName_);
    j.at("isLoop").get_to(t.audioClip_.isLoop_);
    j.at("valume").get_to(t.audioClip_.valume_);
    j.at("isPlay").get_to(t.audioClip_.isPlay_);
}
