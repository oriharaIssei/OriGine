#pragma once

/// microsoft
#include <wrl.h>
/// api
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
/// stl
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace OriGine {

/// <summary>
/// ウェブカメラデバイス1台分の識別情報。
/// </summary>
struct WebCameraDeviceInfo {
    std::wstring id;
    std::wstring name;
};

/// <summary>
/// 取得済みの1映像フレーム（トップダウン BGRA を想定）。
/// </summary>
struct CameraFrame {
    const uint8_t* data = nullptr;
    uint32_t width      = 0;
    uint32_t height     = 0;
    uint32_t stride     = 0;
    uint32_t dataSize   = 0;
};

using CameraFrameCallback = std::function<void(const CameraFrame& frame)>;

/// <summary>
/// Media Foundation の IMFSourceReader を用いてウェブカメラ映像をキャプチャするクラス。
/// RGB32(BGRA)での取得を要求し、デバイスがNV12しか出力しない場合は内部でRGB変換する。
/// 最新フレームはコールバック通知と GetLatestFrame() でのポーリング取得の両方に対応する。
/// </summary>
class WebCamera {
public:
    WebCamera();
    ~WebCamera();

    /// <summary>
    /// Media Foundation を起動する（アプリ全体で一度だけ呼ぶ）。
    /// </summary>
    static void StaticInitialize();
    /// <summary>
    /// Media Foundation を終了する。
    /// </summary>
    static void StaticFinalize();

    /// <summary>
    /// 利用可能なビデオキャプチャデバイスを列挙する。
    /// </summary>
    static std::vector<WebCameraDeviceInfo> EnumerateDevices();

    /// <summary>
    /// 指定デバイス（空ならデフォルト）を開き、指定解像度でのRGB32出力を試みる。
    /// 要求解像度が使えない場合はデバイス既定にフォールバックする。
    /// </summary>
    /// <param name="deviceId">デバイスのシンボリックリンク。空文字列で先頭デバイスを使用</param>
    /// <param name="requestWidth">要求する幅（px）</param>
    /// <param name="requestHeight">要求する高さ（px）</param>
    /// <returns>成功したら true</returns>
    bool Open(const std::wstring& deviceId = L"", uint32_t requestWidth = 640, uint32_t requestHeight = 480);
    void Close();

    /// <summary>
    /// キャプチャスレッドを起動し、フレーム取得を開始する。
    /// </summary>
    bool StartCapture();
    /// <summary>
    /// キャプチャスレッドを停止する。
    /// </summary>
    void StopCapture();

    void SetFrameCallback(CameraFrameCallback callback);

    bool IsCapturing() const { return isCapturing_; }
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    bool GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight);

private:
    /// <summary>
    /// IMFSourceReaderからサンプルを読み出し、必要ならNV12→BGRA変換や上下反転を行って
    /// 最新フレームを更新し、コールバックへ通知するワーカースレッド本体。
    /// </summary>
    void CaptureThread();

    Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader_;

    uint32_t width_  = 0;
    uint32_t height_ = 0;
    uint32_t stride_ = 0;
    bool bottomUp_   = false;
    GUID outputSubtype_ = {};

    std::atomic<bool> isCapturing_{false};
    std::thread captureThread_;

    CameraFrameCallback frameCallback_;
    std::mutex callbackMutex_;

    std::mutex frameMutex_;
    std::vector<uint8_t> latestFrame_;
};

} // namespace OriGine
