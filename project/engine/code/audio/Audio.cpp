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
#include "myGui/MyGui.h"
#endif

#pragma comment(lib, "xaudio2.lib")

Microsoft::WRL::ComPtr<IXAudio2> Audio::xAudio2_;
IXAudio2MasteringVoice* Audio::masterVoice_;

void Audio::StaticInitialize() {
    LOG_DEBUG("Start Static Initialize Audio");
    HRESULT result;

    //===================================================================
    // XAudio2 エンジンインスタンス 作成
    //===================================================================
    result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create XAudio2 engine: " + result);
        assert(false);
    }
    //===================================================================
    // MasteringVoice 作成
    //===================================================================
    result = xAudio2_->CreateMasteringVoice(&masterVoice_);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create mastering voice: " + result);
        assert(false);
    }

    //===================================================================
    // XAudio2 のバージョンを取得
    //===================================================================

    LOG_DEBUG("Complete Static Initialize Audio");
}

void Audio::StaticFinalize() {
    masterVoice_->DestroyVoice();
    xAudio2_.Reset();
}

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
        LOG_CRITICAL("Failed to create source voice: " + result);
        assert(false);
    }

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.valume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = audioClip_.data_.pBuffer;
    buffer.AudioBytes     = audioClip_.data_.bufferSize;
    buffer.Flags          = XAUDIO2_END_OF_STREAM;

    result = pSourceVoice_->SubmitSourceBuffer(&buffer);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to submit source buffer: " + result);
        assert(false);
    }

    result = pSourceVoice_->Start();
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to start source voice: " + result);
        assert(false);
    }
}

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
        LOG_CRITICAL("Failed to create source voice: " + result);
        assert(false);
    }

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.valume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = audioClip_.data_.pBuffer;
    buffer.AudioBytes     = audioClip_.data_.bufferSize;
    buffer.Flags          = XAUDIO2_END_OF_STREAM;
    buffer.LoopBegin      = 0;
    buffer.LoopLength     = 0;
    buffer.LoopCount      = XAUDIO2_LOOP_INFINITE;

    result = pSourceVoice_->SubmitSourceBuffer(&buffer);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to submit source buffer: " + result);
        assert(false);
    }

    result = pSourceVoice_->Start();
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to start source voice: " + result);
        assert(false);
    }
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
            EditorGroup::getInstance()->pushCommand(std::make_unique<SetterCommand<std::string>>(&fileName_, kApplicationResourceDirectory + "/" + directory + "/" + fileName_));

            audioClip_.data_ = LoadWave(kApplicationResourceDirectory + "/" + directory + "/" + fileName_);

            if (audioClip_.data_.pBuffer) {
                isEdit = true;
            }
        }
    }

    ImGui::Text("File:%s", fileName_.c_str());
    isEdit |= CheckBoxCommand("Loop", audioClip_.isLoop_);
    isEdit |= SlideCommand("Volume", audioClip_.valume_, 0.0f, 2.0f);
    isEdit |= CheckBoxCommand("Play", audioClip_.isPlay_);

    if (ImGui::Button("Test Play")) {
        Play();
    }

    return isEdit;
#else
    return false;
#endif // _DEBUG
}

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
