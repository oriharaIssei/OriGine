#include "Mp4Recorder.h"

/// engine
#include "logger/Logger.h"

/// api
#include <mferror.h>
#include <mftransform.h>
/// stl
#include <algorithm>
#include <vector>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace OriGine;
using Microsoft::WRL::ComPtr;

namespace {

constexpr LONGLONG kOneSecond100ns = 10'000'000LL;

// AAC MFT が受け付ける平均バイト/秒は離散値。要求値を最も近い許容値へ丸める。
uint32_t NearestAacBytesPerSecond(uint32_t bitrate) {
    static const uint32_t kValid[] = {12000u, 16000u, 20000u, 24000u}; // 96/128/160/192 kbps
    const uint32_t requested       = bitrate / 8u;
    uint32_t best                  = kValid[0];
    uint32_t bestDiff              = 0xFFFFFFFFu;
    for (uint32_t v : kValid) {
        const uint32_t diff = requested > v ? requested - v : v - requested;
        if (diff < bestDiff) {
            bestDiff = diff;
            best     = v;
        }
    }
    return best;
}

// 文字列パスを UTF-16 へ。
std::wstring Widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

} // namespace

Mp4Recorder::Mp4Recorder() {}

Mp4Recorder::~Mp4Recorder() {
    Close();
}

void Mp4Recorder::Fail(const std::string& msg, HRESULT hr) {
    lastError_ = msg;
    LOG_ERROR("Mp4Recorder: {} (0x{:08X})", msg, static_cast<uint32_t>(hr));
}

bool Mp4Recorder::Open(const std::string& mp4Path, const Mp4VideoConfig& video, const Mp4AudioConfig& audio) {
    Close();

    if (video.width == 0 || video.height == 0) {
        Fail("invalid video size", E_INVALIDARG);
        return false;
    }

    video_    = video;
    audio_    = audio;
    hasAudio_ = audio.enabled && audio.channels > 0 && audio.sampleRate > 0;

    // .mp4 コンテナを明示するため MFCreateAttributes で SinkWriter に指示する。
    ComPtr<IMFAttributes> attributes;
    HRESULT hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) {
        Fail("MFCreateAttributes failed", hr);
        return false;
    }
    attributes->SetUINT32(MF_SINK_WRITER_DISABLE_THROTTLING, TRUE);

    const std::wstring wpath = Widen(mp4Path);
    hr = MFCreateSinkWriterFromURL(wpath.c_str(), nullptr, attributes.Get(), &writer_);
    if (FAILED(hr)) {
        Fail("MFCreateSinkWriterFromURL failed", hr);
        return false;
    }

    if (!ConfigureVideoStream()) return false;
    if (hasAudio_ && !ConfigureAudioStream()) return false;

    hr = writer_->BeginWriting();
    if (FAILED(hr)) {
        Fail("BeginWriting failed", hr);
        writer_.Reset();
        return false;
    }

    startTime100ns_  = 0;
    clockStarted_    = false;
    audioBaseTime_   = -1;
    audioFramesDone_ = 0;
    isRecording_     = true;

    LOG_DEBUG("Mp4Recorder started: {} ({}x{}@{}fps, audio={})",
              mp4Path, video_.width, video_.height, video_.fps, hasAudio_);
    return true;
}

bool Mp4Recorder::ConfigureVideoStream() {
    // 出力 (H.264)
    ComPtr<IMFMediaType> outType;
    HRESULT hr = MFCreateMediaType(&outType);
    if (FAILED(hr)) { Fail("video out MFCreateMediaType failed", hr); return false; }

    outType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    outType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    outType->SetUINT32(MF_MT_AVG_BITRATE, video_.bitrate);
    outType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(outType.Get(), MF_MT_FRAME_SIZE, video_.width, video_.height);
    MFSetAttributeRatio(outType.Get(), MF_MT_FRAME_RATE, video_.fps, 1);
    MFSetAttributeRatio(outType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

    hr = writer_->AddStream(outType.Get(), &videoStream_);
    if (FAILED(hr)) { Fail("video AddStream failed", hr); return false; }

    // 入力 (RGB32 / BGRA, トップダウン)
    ComPtr<IMFMediaType> inType;
    hr = MFCreateMediaType(&inType);
    if (FAILED(hr)) { Fail("video in MFCreateMediaType failed", hr); return false; }

    inType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    inType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    inType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    // 正の stride = トップダウン。WebCamera は上下反転済みのトップダウン BGRA を渡す。
    inType->SetUINT32(MF_MT_DEFAULT_STRIDE, video_.width * 4);
    MFSetAttributeSize(inType.Get(), MF_MT_FRAME_SIZE, video_.width, video_.height);
    MFSetAttributeRatio(inType.Get(), MF_MT_FRAME_RATE, video_.fps, 1);
    MFSetAttributeRatio(inType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

    hr = writer_->SetInputMediaType(videoStream_, inType.Get(), nullptr);
    if (FAILED(hr)) { Fail("video SetInputMediaType failed", hr); return false; }

    return true;
}

bool Mp4Recorder::ConfigureAudioStream() {
    if (audio_.sampleRate != 44100 && audio_.sampleRate != 48000) {
        LOG_WARN("Mp4Recorder: AAC prefers 44100/48000Hz, got {}Hz", audio_.sampleRate);
    }
    const uint16_t channels = (std::min)(audio_.channels, static_cast<uint16_t>(2));

    // 出力 (AAC)
    ComPtr<IMFMediaType> outType;
    HRESULT hr = MFCreateMediaType(&outType);
    if (FAILED(hr)) { Fail("audio out MFCreateMediaType failed", hr); return false; }

    outType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    outType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
    outType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    outType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_.sampleRate);
    outType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
    outType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, NearestAacBytesPerSecond(audio_.bitrate));

    hr = writer_->AddStream(outType.Get(), &audioStream_);
    if (FAILED(hr)) { Fail("audio AddStream failed", hr); return false; }

    // 入力 (16bit PCM)
    ComPtr<IMFMediaType> inType;
    hr = MFCreateMediaType(&inType);
    if (FAILED(hr)) { Fail("audio in MFCreateMediaType failed", hr); return false; }

    const uint32_t blockAlign = channels * 2u;
    inType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    inType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    inType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    inType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_.sampleRate);
    inType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
    inType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, blockAlign);
    inType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, audio_.sampleRate * blockAlign);

    hr = writer_->SetInputMediaType(audioStream_, inType.Get(), nullptr);
    if (FAILED(hr)) { Fail("audio SetInputMediaType failed", hr); return false; }

    audio_.channels = channels;
    return true;
}

LONGLONG Mp4Recorder::NextTimestamp100ns() {
    const LONGLONG now = MFGetSystemTime();
    if (!clockStarted_) {
        startTime100ns_ = now;
        clockStarted_   = true;
        return 0;
    }
    const LONGLONG ts = now - startTime100ns_;
    return ts < 0 ? 0 : ts;
}

bool Mp4Recorder::WriteVideoFrameBGRA(const uint8_t* bgra, uint32_t byteSize) {
    if (!bgra) return false;

    std::lock_guard<std::mutex> lock(writeMutex_);
    if (!isRecording_ || !writer_) return false;

    const uint32_t width      = video_.width;
    const uint32_t height     = video_.height;
    const uint32_t destStride = width * 4u;
    const uint32_t needed     = destStride * height;
    if (byteSize < needed || height == 0) {
        Fail("video frame too small", E_INVALIDARG);
        return false;
    }
    // 行パディングを考慮したソース stride。
    const uint32_t srcStride = byteSize / height;

    ComPtr<IMFMediaBuffer> buffer;
    HRESULT hr = MFCreateMemoryBuffer(needed, &buffer);
    if (FAILED(hr)) { Fail("video MFCreateMemoryBuffer failed", hr); return false; }

    BYTE* dst        = nullptr;
    DWORD maxLen     = 0;
    hr = buffer->Lock(&dst, &maxLen, nullptr);
    if (FAILED(hr)) { Fail("video buffer Lock failed", hr); return false; }

    // トップダウン → トップダウン。stride が違えば MFCopyImage が幅ぶんだけ詰めてコピーする。
    hr = MFCopyImage(dst, destStride, bgra, srcStride, destStride, height);
    buffer->Unlock();
    if (FAILED(hr)) { Fail("MFCopyImage failed", hr); return false; }

    buffer->SetCurrentLength(needed);

    ComPtr<IMFSample> sample;
    hr = MFCreateSample(&sample);
    if (FAILED(hr)) { Fail("video MFCreateSample failed", hr); return false; }
    sample->AddBuffer(buffer.Get());

    sample->SetSampleTime(NextTimestamp100ns());
    sample->SetSampleDuration(kOneSecond100ns / (video_.fps > 0 ? video_.fps : 30));

    hr = writer_->WriteSample(videoStream_, sample.Get());
    if (FAILED(hr)) { Fail("video WriteSample failed", hr); return false; }
    return true;
}

bool Mp4Recorder::WriteAudioFloat(const float* interleaved, uint32_t frameCount, uint32_t channels) {
    if (!interleaved || frameCount == 0) return false;

    std::lock_guard<std::mutex> lock(writeMutex_);
    if (!isRecording_ || !writer_ || !hasAudio_) return false;

    const uint32_t outChannels = audio_.channels;
    const uint32_t sampleCount = frameCount * outChannels;
    const uint32_t byteLen     = sampleCount * sizeof(int16_t);

    ComPtr<IMFMediaBuffer> buffer;
    HRESULT hr = MFCreateMemoryBuffer(byteLen, &buffer);
    if (FAILED(hr)) { Fail("audio MFCreateMemoryBuffer failed", hr); return false; }

    BYTE* dst    = nullptr;
    DWORD maxLen = 0;
    hr = buffer->Lock(&dst, &maxLen, nullptr);
    if (FAILED(hr)) { Fail("audio buffer Lock failed", hr); return false; }

    int16_t* out = reinterpret_cast<int16_t*>(dst);
    for (uint32_t f = 0; f < frameCount; ++f) {
        for (uint32_t c = 0; c < outChannels; ++c) {
            // 入力チャンネル数が要求と違っても安全に読む（不足分は先頭チャンネル or 0）。
            const uint32_t srcCh = channels > 0 ? (c < channels ? c : 0) : 0;
            float s = channels > 0 ? interleaved[f * channels + srcCh] : 0.0f;
            s = (std::max)(-1.0f, (std::min)(1.0f, s));
            out[f * outChannels + c] = static_cast<int16_t>(s * 32767.0f);
        }
    }
    buffer->Unlock();
    buffer->SetCurrentLength(byteLen);

    ComPtr<IMFSample> sample;
    hr = MFCreateSample(&sample);
    if (FAILED(hr)) { Fail("audio MFCreateSample failed", hr); return false; }
    sample->AddBuffer(buffer.Get());

    // 音声はサンプル数から連続した時間軸を作り、基準時計へのオフセットで映像と揃える。
    if (audioBaseTime_ < 0) {
        const LONGLONG now = MFGetSystemTime();
        if (!clockStarted_) {
            startTime100ns_ = now;
            clockStarted_   = true;
        }
        audioBaseTime_ = now - startTime100ns_;
        if (audioBaseTime_ < 0) audioBaseTime_ = 0;
    }
    const LONGLONG ts = audioBaseTime_ + (static_cast<LONGLONG>(audioFramesDone_) * kOneSecond100ns) / audio_.sampleRate;
    const LONGLONG dur = (static_cast<LONGLONG>(frameCount) * kOneSecond100ns) / audio_.sampleRate;
    sample->SetSampleTime(ts);
    sample->SetSampleDuration(dur);
    audioFramesDone_ += frameCount;

    hr = writer_->WriteSample(audioStream_, sample.Get());
    if (FAILED(hr)) { Fail("audio WriteSample failed", hr); return false; }
    return true;
}

void Mp4Recorder::Close() {
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (!writer_) {
        isRecording_ = false;
        return;
    }
    isRecording_ = false;
    HRESULT hr = writer_->Finalize();
    if (FAILED(hr)) {
        Fail("Finalize failed", hr);
    }
    writer_.Reset();
    LOG_DEBUG("Mp4Recorder closed");
}
