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
    /// <summary>
    /// 録画時のエンコード設定.
    /// </summary>
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

    /// <summary>
    /// ウェブカメラとマイクのコールバックを Mp4Recorder に結線し、録画を開始する.
    /// camera は Open 済み（幅/高さ確定）である必要がある。mic は recordAudio 時のみ必須。
    /// </summary>
    /// <param name="camera">Open 済みのウェブカメラ</param>
    /// <param name="microphone">マイク入力（recordAudio が false の場合は未使用）</param>
    /// <param name="mp4Path">出力先 mp4 ファイルパス</param>
    /// <param name="config">エンコード設定</param>
    /// <returns>開始に成功したら true</returns>
    bool Start(WebCamera* camera, Microphone* microphone, const std::string& mp4Path, const Config& config = {});
    /// <summary>
    /// 録画を停止し、結線したコールバックを解除する.
    /// </summary>
    void Stop();

    /// <summary> 現在録画中かどうかを取得する. </summary>
    bool IsRecording() const { return active_; }
    /// <summary> 直近に発生したエラーメッセージを取得する. </summary>
    const std::string& GetLastError() const { return recorder_.GetLastError(); }

private:
    Mp4Recorder recorder_;
    WebCamera*  camera_     = nullptr; // 結線対象のウェブカメラ（非所有）
    Microphone* microphone_ = nullptr; // 結線対象のマイク（非所有）
    std::atomic<bool> active_{false}; // 録画中フラグ
};

} // namespace OriGine
