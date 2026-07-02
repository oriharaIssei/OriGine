#include "MediaRecorder.h"

/// engine
#include "logger/Logger.h"
#include "mediaCapture/Microphone.h"
#include "mediaCapture/WebCamera.h"

using namespace OriGine;

bool MediaRecorder::Start(WebCamera* camera, Microphone* microphone, const std::string& mp4Path, const Config& config) {
    if (active_) {
        LOG_WARN("MediaRecorder: already recording");
        return false;
    }
    if (!camera) {
        LOG_ERROR("MediaRecorder: camera is null");
        return false;
    }

    const uint32_t width  = camera->GetWidth();
    const uint32_t height = camera->GetHeight();
    if (width == 0 || height == 0) {
        LOG_ERROR("MediaRecorder: camera not opened (size 0). Call camera->Open() first");
        return false;
    }

    Mp4VideoConfig videoCfg;
    videoCfg.width   = width;
    videoCfg.height  = height;
    videoCfg.fps     = config.fps;
    videoCfg.bitrate = config.videoBitrate;

    Mp4AudioConfig audioCfg;
    audioCfg.enabled = config.recordAudio && microphone != nullptr;
    if (audioCfg.enabled) {
        const MicrophoneFormat& fmt = microphone->GetFormat();
        audioCfg.sampleRate = fmt.sampleRate;
        audioCfg.channels   = fmt.channels;
        audioCfg.bitrate    = config.audioBitrate;
    }

    if (!recorder_.Open(mp4Path, videoCfg, audioCfg)) {
        LOG_ERROR("MediaRecorder: failed to open mp4: {}", recorder_.GetLastError());
        return false;
    }

    camera_     = camera;
    microphone_ = audioCfg.enabled ? microphone : nullptr;
    active_     = true;

    camera_->SetFrameCallback([this](const CameraFrame& frame) {
        if (active_ && frame.data) {
            recorder_.WriteVideoFrameBGRA(frame.data, frame.dataSize);
        }
    });

    if (microphone_) {
        microphone_->SetDataCallback([this](const float* data, uint32_t frameCount, uint32_t channels) {
            if (active_) {
                recorder_.WriteAudioFloat(data, frameCount, channels);
            }
        });
    }

    LOG_DEBUG("MediaRecorder started: {}", mp4Path);
    return true;
}

void MediaRecorder::Stop() {
    if (!active_) return;
    active_ = false;

    // これ以降のフレームがレコーダへ渡らないようコールバックを解除する。
    if (camera_) {
        camera_->SetFrameCallback(nullptr);
        camera_ = nullptr;
    }
    if (microphone_) {
        microphone_->SetDataCallback(nullptr);
        microphone_ = nullptr;
    }

    recorder_.Close();
    LOG_DEBUG("MediaRecorder stopped");
}
