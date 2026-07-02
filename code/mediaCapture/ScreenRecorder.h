#pragma once

/// engine
#include "mediaCapture/Mp4Recorder.h"
/// stl
#include <atomic>
#include <cstdint>
#include <string>

namespace OriGine {

class ScreenCapture;
class SystemAudioCapture;

// 指定スクリーン（ScreenCapture）と PC 内部音（SystemAudioCapture のループバック）を
// Mp4Recorder に結線し、画面録画＋システム音声を mp4 に書き出す薄いグルー。
//
// 使い方:
//   screen->Open(dxDevice, dxCommand, monitorIndex); screen->StartCapture();
//   sysAudio->Open();                                sysAudio->StartCapture();
//   ScreenRecorder rec;
//   rec.Start(screen, sysAudio, "screen.mp4");
//   ... 録画したい間キャプチャを回す ...
//   rec.Stop();
//
// 注意: Start は screen->SetFrameCallback / sysAudio->SetDataCallback を上書きし、
// Stop で解除する。録画中は他用途のコールバックと排他になる。
class ScreenRecorder {
public:
    struct Config {
        uint32_t fps          = 30;
        uint32_t videoBitrate = 8'000'000; // bps（画面は解像度が大きいので既定を高めに）
        uint32_t audioBitrate = 128'000;   // bps
        bool     recordAudio  = true;
    };

    ScreenRecorder()  = default;
    ~ScreenRecorder() { Stop(); }

    ScreenRecorder(const ScreenRecorder&)            = delete;
    ScreenRecorder& operator=(const ScreenRecorder&) = delete;

    // screen は Open 済み（幅/高さ確定）である必要がある。systemAudio は recordAudio 時のみ必須。
    bool Start(ScreenCapture* screen, SystemAudioCapture* systemAudio, const std::string& mp4Path, const Config& config = {});
    void Stop();

    bool IsRecording() const { return active_; }
    const std::string& GetLastError() const { return recorder_.GetLastError(); }

private:
    Mp4Recorder        recorder_;
    ScreenCapture*     screen_      = nullptr;
    SystemAudioCapture* systemAudio_ = nullptr;
    std::atomic<bool>  active_{false};
};

} // namespace OriGine
