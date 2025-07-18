#include "Audio.h"

/// stl
#include <cassert>
#include <fstream>
#include <iostream>

/// engine
#define RESOURCE_DIRECTORY
#include "ECSManager.h"
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

#pragma region "Audio"
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
    //===================================================================
    result = xAudio2_->CreateMasteringVoice(&masterVoice_);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create mastering voice: {}", result);
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
        LOG_CRITICAL("Failed to create source voice: {}", result);
        assert(false);
    }

    // 音量を設定
    pSourceVoice_->SetVolume(audioClip_.volume_);

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData     = audioClip_.data_.pBuffer;
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
    buffer.pAudioData     = audioClip_.data_.pBuffer;
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

void Audio::Pause() {
    pSourceVoice_->Stop(0);
}

bool Audio::Edit() {
#ifdef _DEBUG
    bool isEdit = false;
    if (ImGui::Button("LoadFile")) {
        std::string directory;
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName_, {"wav"})) {
            EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<std::string>>(&fileName_, kApplicationResourceDirectory + "/" + directory + "/" + fileName_));

            audioClip_.data_ = LoadWave(kApplicationResourceDirectory + "/" + directory + "/" + fileName_);

            if (audioClip_.data_.pBuffer) {
                isEdit = true;
            }
        }
    }

    ImGui::Text("File:%s", fileName_.c_str());
    isEdit |= CheckBoxCommand("Loop", audioClip_.isLoop_);
    isEdit |= SlideGuiCommand("Volume", audioClip_.volume_, 0.0f, 2.0f);
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
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: {}", fileName);
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

    bool foundFmt     = false;
    bool foundData    = false;
    DWORD dataSize    = 0;
    BYTE* pDataBuffer = nullptr;

    while (file.read(reinterpret_cast<char*>(&chunk), sizeof(chunk))) {
        std::streampos nextChunk = file.tellg();
        nextChunk += chunk.size;

        if (strncmp(chunk.id, "fmt ", 4) == 0) {
            foundFmt = true;
            file.read(reinterpret_cast<char*>(&format.fmt), chunk.size);
        } else if (strncmp(chunk.id, "data", 4) == 0) {
            foundData   = true;
            pDataBuffer = new BYTE[chunk.size];
            file.read(reinterpret_cast<char*>(pDataBuffer), chunk.size);
            dataSize = chunk.size;
        } else {
            // 未使用のチャンクはスキップ
            file.seekg(chunk.size, std::ios::cur);
        }

        file.seekg(nextChunk);
    }

    if (!foundFmt || !foundData) {
        LOG_ERROR("Required fmt or data chunk not found");
        delete[] pDataBuffer;
        return {};
    }

    SoundData soundData{};
    soundData.wfex       = format.fmt;
    soundData.pBuffer    = pDataBuffer;
    soundData.bufferSize = dataSize;

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
    j["volume"]   = t.audioClip_.volume_;
}

void from_json(const nlohmann::json& j, Audio& t) {
    j.at("fileName").get_to(t.fileName_);
    j.at("isLoop").get_to(t.audioClip_.isLoop_);
    j.at("volume").get_to(t.audioClip_.volume_);
}

#pragma endregion "Audio"

AudioInitializeSystem::AudioInitializeSystem() : ISystem(SystemType::Initialize) {};

AudioInitializeSystem::~AudioInitializeSystem() {}

void AudioInitializeSystem::Initialize() {}
void AudioInitializeSystem::Finalize() {}

void AudioInitializeSystem::UpdateEntity(GameEntity* entity) {
    int32_t entityIndex = 0;

    while (true) {
        Audio* audio = getComponent<Audio>(entity, entityIndex);
        if (!audio) {
            return;
        }

        // 再生
        audio->Play();

        entityIndex++;
    }
}
