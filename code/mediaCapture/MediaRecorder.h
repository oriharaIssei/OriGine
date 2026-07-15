#pragma once

/// engine
#include "mediaCapture/Mp4Recorder.h"
/// stl
#include <atomic>
#include <cstdint>
#include <string>

namespace OriGine {

class WebCamera;
class Microphone;

// WebCamera + Microphone を Mp4Recorder に結線し、
// 「ウェブカメラとマイクの入力を mp4 に変換する」機能をワンストップで提供する薄いグルー。
//
// 使い方:
//   camera->Open(...); camera->StartCapture();
//   mic->Open(...);    mic->StartCapture();
//   MediaRecorder rec;
//   rec.Start(camera, mic, "capture.mp4");
//   ... （録画したい間キャプチャを回す）...
//   rec.Stop();
//
// 注意: Start は camera->SetFrameCallback / mic->SetDataCallback を上書きし、
// Stop でコールバックを解除する。録画中は他用途のコールバックと排他になる。
class MediaRecorder {
public:
    struct Config {
        uint32_t fps          = 30;
        uint32_t videoBitrate = 4'000'000; // bps
        uint32_t audioBitrate = 128'000;   // bps
        bool     recordAudio  = true;
    };

    MediaRecorder()  = default;
    ~MediaRecorder() { Stop(); }

    MediaRecorder(const MediaRecorder&)            = delete;
    MediaRecorder& operator=(const MediaRecorder&) = delete;

    // camera は Open 済み（幅/高さ確定）である必要がある。mic は recordAudio 時のみ必須。
    bool Start(WebCamera* camera, Microphone* microphone, const std::string& mp4Path, const Config& config = {});
    void Stop();

    bool IsRecording() const { return active_; }
    const std::string& GetLastError() const { return recorder_.GetLastError(); }

private:
    Mp4Recorder recorder_;
    WebCamera*  camera_     = nullptr;
    Microphone* microphone_ = nullptr;
    std::atomic<bool> active_{false};
};

} // namespace OriGine
