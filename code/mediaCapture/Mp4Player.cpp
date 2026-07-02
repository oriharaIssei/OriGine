#include "Mp4Player.h"

/// engine
#include "logger/Logger.h"

/// api
#include <mferror.h>
/// stl
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "xaudio2.lib")

using namespace OriGine;
using Microsoft::WRL::ComPtr;

namespace {
constexpr LONGLONG kOneSecond100ns = 10'000'000LL;
constexpr size_t   kMaxQueuedFrames = 8;   // 映像先読みバッファ上限
constexpr uint32_t kMaxQueuedAudio  = 32;  // 音声送出バッファ上限（XAudio2 max 64）

std::wstring Widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), out.data(), len);
    return out;
}
} // namespace

Mp4Player::Mp4Player() {
    QueryPerformanceFrequency(&qpcFreq_);
}

Mp4Player::~Mp4Player() {
    Close();
}

void Mp4Player::Fail(const std::string& msg, HRESULT hr) {
    lastError_ = msg;
    LOG_ERROR("Mp4Player: {} (0x{:08X})", msg, static_cast<uint32_t>(hr));
}

bool Mp4Player::Open(const std::string& mp4Path) {
    Close();

    ComPtr<IMFAttributes> attr;
    HRESULT hr = MFCreateAttributes(&attr, 1);
    if (FAILED(hr)) { Fail("MFCreateAttributes failed", hr); return false; }
    // デコーダに RGB32 への色変換を許可する。
    attr->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

    const std::wstring wpath = Widen(mp4Path);
    hr = MFCreateSourceReaderFromURL(wpath.c_str(), attr.Get(), &reader_);
    if (FAILED(hr)) { Fail("MFCreateSourceReaderFromURL failed", hr); return false; }

    if (!ConfigureStreams()) { Close(); return false; }
    if (hasAudio_ && !CreateAudioOutput()) { Close(); return false; }

    // 総再生時間
    PROPVARIANT var;
    PropVariantInit(&var);
    if (SUCCEEDED(reader_->GetPresentationAttribute(
            static_cast<DWORD>(MF_SOURCE_READER_MEDIASOURCE), MF_PD_DURATION, &var))) {
        duration100ns_ = var.uhVal.QuadPart;
    }
    PropVariantClear(&var);

    // クロック初期化
    segmentStartPts_   = 0;
    baseSamples_       = 0;
    playedAccum100ns_  = 0;
    decodeFinished_    = false;
    state_             = State::Idle;

    running_       = true;
    decodeThread_  = std::thread(&Mp4Player::DecodeThread, this);
    presentThread_ = std::thread(&Mp4Player::PresentThread, this);

    LOG_DEBUG("Mp4Player opened: {} ({}x{}, audio={}, {:.2f}s)",
              mp4Path, width_, height_, hasAudio_, GetDuration());
    return true;
}

bool Mp4Player::ConfigureStreams() {
    // 各ストリームの major type を調べ、映像/音声の実インデックスを特定する。
    for (DWORD i = 0;; ++i) {
        ComPtr<IMFMediaType> nativeType;
        HRESULT hr = reader_->GetNativeMediaType(i, 0, &nativeType);
        if (hr == MF_E_INVALIDSTREAMNUMBER) break;
        if (FAILED(hr)) continue;

        GUID major = {};
        nativeType->GetGUID(MF_MT_MAJOR_TYPE, &major);
        if (major == MFMediaType_Video && !hasVideo_) {
            videoStreamIndex_ = i;
            hasVideo_ = true;
        } else if (major == MFMediaType_Audio && !hasAudio_) {
            audioStreamIndex_ = i;
            hasAudio_ = true;
        }
    }

    if (!hasVideo_) {
        Fail("no video stream in mp4", E_FAIL);
        return false;
    }

    // 映像出力を RGB32 に設定
    ComPtr<IMFMediaType> videoOut;
    MFCreateMediaType(&videoOut);
    videoOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    videoOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    HRESULT hr = reader_->SetCurrentMediaType(videoStreamIndex_, nullptr, videoOut.Get());
    if (FAILED(hr)) { Fail("video SetCurrentMediaType(RGB32) failed", hr); return false; }

    ComPtr<IMFMediaType> curVideo;
    hr = reader_->GetCurrentMediaType(videoStreamIndex_, &curVideo);
    if (FAILED(hr)) { Fail("GetCurrentMediaType(video) failed", hr); return false; }
    MFGetAttributeSize(curVideo.Get(), MF_MT_FRAME_SIZE, &width_, &height_);

    UINT32 strideVal = 0;
    if (SUCCEEDED(curVideo->GetUINT32(MF_MT_DEFAULT_STRIDE, &strideVal))) {
        INT32 s = static_cast<INT32>(strideVal);
        stride_   = static_cast<uint32_t>(std::abs(s));
        bottomUp_ = (s < 0);
    } else {
        stride_   = width_ * 4;
        bottomUp_ = false;
    }

    if (hasAudio_) {
        // 音声出力を PCM16 に設定（サンプルレート/チャンネルはソース既定を採用）
        ComPtr<IMFMediaType> audioOut;
        MFCreateMediaType(&audioOut);
        audioOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        audioOut->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        audioOut->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        hr = reader_->SetCurrentMediaType(audioStreamIndex_, nullptr, audioOut.Get());
        if (FAILED(hr)) {
            LOG_WARN("Mp4Player: audio SetCurrentMediaType(PCM) failed, disabling audio (0x{:08X})",
                     static_cast<uint32_t>(hr));
            hasAudio_ = false;
        } else {
            ComPtr<IMFMediaType> curAudio;
            if (SUCCEEDED(reader_->GetCurrentMediaType(audioStreamIndex_, &curAudio))) {
                curAudio->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &audioRate_);
                curAudio->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &audioChannels_);
            }
            if (audioRate_ == 0 || audioChannels_ == 0) hasAudio_ = false;
        }
    }

    return true;
}

bool Mp4Player::CreateAudioOutput() {
    HRESULT hr = XAudio2Create(&xaudio_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) { Fail("XAudio2Create failed", hr); return false; }

    hr = xaudio_->CreateMasteringVoice(&masterVoice_);
    if (FAILED(hr)) { Fail("CreateMasteringVoice failed", hr); return false; }

    WAVEFORMATEX wfx = {};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = static_cast<WORD>(audioChannels_);
    wfx.nSamplesPerSec  = audioRate_;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = static_cast<WORD>(audioChannels_ * 2);
    wfx.nAvgBytesPerSec = audioRate_ * wfx.nBlockAlign;
    wfx.cbSize          = 0;

    hr = xaudio_->CreateSourceVoice(&sourceVoice_, &wfx);
    if (FAILED(hr)) { Fail("CreateSourceVoice failed", hr); return false; }

    return true;
}

void Mp4Player::Play() {
    if (!reader_) return;
    if (state_ == State::Finished) {
        pendingSeek100ns_ = 0; // 終端からの再生は先頭へ
    }
    {
        std::lock_guard<std::mutex> lock(clockMutex_);
        QueryPerformanceCounter(&playStartQpc_);
    }
    if (sourceVoice_) sourceVoice_->Start(0);
    state_ = State::Playing;
}

void Mp4Player::Pause() {
    if (state_ != State::Playing) return;
    if (sourceVoice_) sourceVoice_->Stop(0);
    if (!hasAudio_) {
        std::lock_guard<std::mutex> lock(clockMutex_);
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        playedAccum100ns_ += (now.QuadPart - playStartQpc_.QuadPart) * kOneSecond100ns / qpcFreq_.QuadPart;
    }
    state_ = State::Paused;
}

void Mp4Player::Stop() {
    if (!reader_) return;
    if (sourceVoice_) sourceVoice_->Stop(0);
    state_ = State::Paused;
    pendingSeek100ns_ = 0;
}

bool Mp4Player::Seek(double seconds) {
    if (!reader_) return false;
    LONGLONG pos = static_cast<LONGLONG>(seconds * 1e7);
    if (pos < 0) pos = 0;
    if (duration100ns_ > 0 && pos > duration100ns_) pos = duration100ns_;
    pendingSeek100ns_ = pos;
    return true;
}

void Mp4Player::ApplySeekLocked(LONGLONG pos100ns) {
    // デコードスレッドから呼ばれる。SourceReader を移動し、出力・クロックをリセットする。
    PROPVARIANT var;
    PropVariantInit(&var);
    var.vt        = VT_I8;
    var.hVal.QuadPart = pos100ns;
    const GUID timeFormat = {}; // 全ゼロ = GUID_NULL = 既定(100ns 単位)。エクスポートシンボル依存を避ける
    reader_->SetCurrentPosition(timeFormat, var);
    PropVariantClear(&var);

    if (sourceVoice_) {
        sourceVoice_->FlushSourceBuffers();
    }
    audioBuffers_.clear();

    {
        std::lock_guard<std::mutex> lock(frameQueueMutex_);
        frameQueue_.clear();
    }

    {
        std::lock_guard<std::mutex> lock(clockMutex_);
        segmentStartPts_ = pos100ns;
        if (hasAudio_ && sourceVoice_) {
            XAUDIO2_VOICE_STATE vs = {};
            sourceVoice_->GetState(&vs, 0);
            baseSamples_ = vs.SamplesPlayed;
        }
        playedAccum100ns_ = pos100ns;
        QueryPerformanceCounter(&playStartQpc_);
    }

    decodeFinished_ = false;
}

void Mp4Player::DecodeThread() {
    while (running_) {
        // 保留中のシーク要求を適用
        LONGLONG seek = pendingSeek100ns_.exchange(-1);
        if (seek >= 0) {
            ApplySeekLocked(seek);
            if (state_ == State::Finished) state_ = State::Paused;
        }

        if (state_ != State::Playing) {
            Sleep(5);
            continue;
        }

        // 先読みバッファが十分あれば読み込みを止めて待つ（バックプレッシャ）
        {
            size_t vq;
            {
                std::lock_guard<std::mutex> lock(frameQueueMutex_);
                vq = frameQueue_.size();
            }
            uint32_t aq = 0;
            if (hasAudio_ && sourceVoice_) {
                XAUDIO2_VOICE_STATE vs = {};
                sourceVoice_->GetState(&vs, 0);
                aq = vs.BuffersQueued;
            }
            if (vq >= kMaxQueuedFrames || aq >= kMaxQueuedAudio) {
                Sleep(3);
                continue;
            }
        }

        if (decodeFinished_) {
            Sleep(5);
            continue;
        }

        DWORD actualIndex = 0;
        DWORD flags       = 0;
        LONGLONG ts       = 0;
        ComPtr<IMFSample> sample;
        HRESULT hr = reader_->ReadSample(
            static_cast<DWORD>(MF_SOURCE_READER_ANY_STREAM), 0,
            &actualIndex, &flags, &ts, &sample);
        if (FAILED(hr)) {
            Sleep(5);
            continue;
        }

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            if (loop_) {
                pendingSeek100ns_ = 0;
            } else {
                decodeFinished_ = true;
            }
            continue;
        }

        if (!sample) continue;

        if (hasAudio_ && actualIndex == audioStreamIndex_) {
            SubmitAudio(sample);
        } else if (actualIndex == videoStreamIndex_) {
            PushVideo(sample, ts);
        }
    }
}

void Mp4Player::SubmitAudio(const ComPtr<IMFSample>& sample) {
    if (!sourceVoice_) return;

    ComPtr<IMFMediaBuffer> buffer;
    if (FAILED(sample->ConvertToContiguousBuffer(&buffer))) return;

    BYTE* data      = nullptr;
    DWORD maxLen    = 0;
    DWORD curLen    = 0;
    if (FAILED(buffer->Lock(&data, &maxLen, &curLen))) return;

    auto owned = std::make_unique<std::vector<uint8_t>>(data, data + curLen);
    buffer->Unlock();

    XAUDIO2_BUFFER xb = {};
    xb.AudioBytes = static_cast<UINT32>(owned->size());
    xb.pAudioData = owned->data();
    if (SUCCEEDED(sourceVoice_->SubmitSourceBuffer(&xb))) {
        audioBuffers_.push_back(std::move(owned));
    }

    // 消費済みバッファを解放（FIFO 消費なので先頭から安全に破棄できる）
    XAUDIO2_VOICE_STATE vs = {};
    sourceVoice_->GetState(&vs, 0);
    while (audioBuffers_.size() > vs.BuffersQueued) {
        audioBuffers_.pop_front();
    }
}

void Mp4Player::PushVideo(const ComPtr<IMFSample>& sample, LONGLONG pts) {
    ComPtr<IMFMediaBuffer> buffer;
    if (FAILED(sample->ConvertToContiguousBuffer(&buffer))) return;

    BYTE* data   = nullptr;
    DWORD maxLen = 0;
    DWORD curLen = 0;
    if (FAILED(buffer->Lock(&data, &maxLen, &curLen))) return;

    const uint32_t rowBytes = width_ * 4;
    DecodedFrame frame;
    frame.pts100ns = pts;
    frame.bgra.resize(rowBytes * height_);

    if (bottomUp_ && curLen >= stride_ * height_) {
        for (uint32_t y = 0; y < height_; ++y) {
            std::memcpy(frame.bgra.data() + y * rowBytes,
                        data + (height_ - 1 - y) * stride_, rowBytes);
        }
    } else if (stride_ == rowBytes && curLen >= rowBytes * height_) {
        std::memcpy(frame.bgra.data(), data, rowBytes * height_);
    } else {
        const uint32_t copyRow = stride_ < rowBytes ? stride_ : rowBytes;
        for (uint32_t y = 0; y < height_; ++y) {
            if (static_cast<DWORD>((y + 1) * stride_) > curLen) break;
            std::memcpy(frame.bgra.data() + y * rowBytes, data + y * stride_, copyRow);
        }
    }
    buffer->Unlock();

    std::lock_guard<std::mutex> lock(frameQueueMutex_);
    frameQueue_.push_back(std::move(frame));
}

LONGLONG Mp4Player::GetMasterPts100ns() {
    if (hasAudio_ && sourceVoice_) {
        XAUDIO2_VOICE_STATE vs = {};
        sourceVoice_->GetState(&vs, 0);
        std::lock_guard<std::mutex> lock(clockMutex_);
        uint64_t played = (vs.SamplesPlayed >= baseSamples_) ? (vs.SamplesPlayed - baseSamples_) : 0;
        return segmentStartPts_ + static_cast<LONGLONG>(played * static_cast<uint64_t>(kOneSecond100ns) / audioRate_);
    }

    std::lock_guard<std::mutex> lock(clockMutex_);
    if (state_ == State::Playing) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return playedAccum100ns_ + (now.QuadPart - playStartQpc_.QuadPart) * kOneSecond100ns / qpcFreq_.QuadPart;
    }
    return playedAccum100ns_;
}

void Mp4Player::PresentThread() {
    while (running_) {
        if (state_ != State::Playing) {
            Sleep(5);
            continue;
        }

        const LONGLONG master = GetMasterPts100ns();

        bool present = false;
        DecodedFrame chosen;
        {
            std::lock_guard<std::mutex> lock(frameQueueMutex_);
            // master に達したフレームまで進める（遅延時は間引いて最新へ）
            while (!frameQueue_.empty() && frameQueue_.front().pts100ns <= master) {
                chosen  = std::move(frameQueue_.front());
                frameQueue_.pop_front();
                present = true;
            }
        }

        if (present) {
            {
                std::lock_guard<std::mutex> lock(frameMutex_);
                latestFrame_ = chosen.bgra;
            }
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (frameCallback_) {
                frameCallback_(chosen.bgra.data(), width_, height_);
            }
        } else {
            // 再生完了判定: デコード終了かつ供給待ちのフレームが無い
            bool empty;
            {
                std::lock_guard<std::mutex> lock(frameQueueMutex_);
                empty = frameQueue_.empty();
            }
            if (decodeFinished_ && empty && !loop_) {
                if (sourceVoice_) {
                    XAUDIO2_VOICE_STATE vs = {};
                    sourceVoice_->GetState(&vs, 0);
                    if (vs.BuffersQueued == 0) {
                        state_ = State::Finished;
                    }
                } else {
                    state_ = State::Finished;
                }
            }
            Sleep(2);
        }
    }
}

double Mp4Player::GetPosition() const {
    return static_cast<double>(const_cast<Mp4Player*>(this)->GetMasterPts100ns()) / 1e7;
}

bool Mp4Player::GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight) {
    std::lock_guard<std::mutex> lock(frameMutex_);
    if (latestFrame_.empty()) return false;
    outBuffer = latestFrame_;
    outWidth  = width_;
    outHeight = height_;
    return true;
}

void Mp4Player::SetFrameCallback(Mp4VideoFrameCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    frameCallback_ = std::move(callback);
}

void Mp4Player::Close() {
    running_ = false;
    state_   = State::Idle;

    if (decodeThread_.joinable())  decodeThread_.join();
    if (presentThread_.joinable()) presentThread_.join();

    if (sourceVoice_) {
        sourceVoice_->Stop(0);
        sourceVoice_->FlushSourceBuffers();
        sourceVoice_->DestroyVoice();
        sourceVoice_ = nullptr;
    }
    audioBuffers_.clear();
    if (masterVoice_) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    xaudio_.Reset();

    reader_.Reset();

    {
        std::lock_guard<std::mutex> lock(frameQueueMutex_);
        frameQueue_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(frameMutex_);
        latestFrame_.clear();
    }

    hasVideo_ = false;
    hasAudio_ = false;
    width_ = height_ = stride_ = 0;
    bottomUp_ = false;
    duration100ns_ = 0;
    audioRate_ = audioChannels_ = 0;
    decodeFinished_ = false;
    pendingSeek100ns_ = -1;
}
