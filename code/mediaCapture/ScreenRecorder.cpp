#include "ScreenRecorder.h"

/// engine
#include "logger/Logger.h"
#include "mediaCapture/ScreenCapture.h"
#include "mediaCapture/SystemAudioCapture.h"

using namespace OriGine;

bool ScreenRecorder::Start(ScreenCapture* screen, SystemAudioCapture* systemAudio, const std::string& mp4Path, const Config& config) {
    if (active_) {
        LOG_WARN("ScreenRecorder: already recording");
        return false;
    }
    if (!screen) {
        LOG_ERROR("ScreenRecorder: screen is null");
        return false;
    }

    const uint32_t width  = screen->GetWidth();
    const uint32_t height = screen->GetHeight();
    if (width == 0 || height == 0) {
        LOG_ERROR("ScreenRecorder: screen not opened (size 0). Call screen->Open() first");
        return false;
    }
    // H.264 は偶数解像度が前提。奇数だとエンコーダ構成に失敗するため弾く。
    if ((width & 1u) != 0 || (height & 1u) != 0) {
        LOG_ERROR("ScreenRecorder: odd screen dimension {}x{} not supported by H.264", width, height);
        return false;
    }

    Mp4VideoConfig videoCfg;
    videoCfg.width   = width;
    videoCfg.height  = height;
    videoCfg.fps     = config.fps;
    videoCfg.bitrate = config.videoBitrate;

    Mp4AudioConfig audioCfg;
    audioCfg.enabled = config.recordAudio && systemAudio != nullptr;
    if (audioCfg.enabled) {
        const SystemAudioFormat& fmt = systemAudio->GetFormat();
        audioCfg.sampleRate = fmt.sampleRate;
        audioCfg.channels   = fmt.channels;
        audioCfg.bitrate    = config.audioBitrate;
    }

    if (!recorder_.Open(mp4Path, videoCfg, audioCfg)) {
        LOG_ERROR("ScreenRecorder: failed to open mp4: {}", recorder_.GetLastError());
        return false;
    }

    screen_      = screen;
    systemAudio_ = audioCfg.enabled ? systemAudio : nullptr;
    active_      = true;

    screen_->SetFrameCallback([this](const ScreenFrame& frame) {
        if (active_ && frame.cpuData) {
            recorder_.WriteVideoFrameBGRA(frame.cpuData, frame.dataSize);
        }
    });

    if (systemAudio_) {
        systemAudio_->SetDataCallback([this](const float* data, uint32_t frameCount, uint32_t channels) {
            if (active_) {
                recorder_.WriteAudioFloat(data, frameCount, channels);
            }
        });
    }

    LOG_DEBUG("ScreenRecorder started: {} ({}x{})", mp4Path, width, height);
    return true;
}

void ScreenRecorder::Stop() {
    if (!active_) return;
    active_ = false;

    if (screen_) {
        screen_->SetFrameCallback(nullptr);
        screen_ = nullptr;
    }
    if (systemAudio_) {
        systemAudio_->SetDataCallback(nullptr);
        systemAudio_ = nullptr;
    }

    recorder_.Close();
    LOG_DEBUG("ScreenRecorder stopped");
}
