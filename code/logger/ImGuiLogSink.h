#pragma once

/// stl
#include <memory>
#include <mutex>
#include <string>

#include <vector>

/// externals
#include <spdlog/sinks/sink.h>
#include <spdlog/spdlog.h>

namespace OriGine {

/// <summary>
/// ImGui 表示用のカスタム spdlog シンク.
/// ログメッセージをメモリ（std::vector）に蓄積し、後で GUI から参照できるようにする.
/// </summary>
class ImGuiLogSink : public spdlog::sinks::sink {
public:
    ImGuiLogSink()           = default;
    ~ImGuiLogSink() override = default;

    /// <summary>
    /// spdlog から呼び出されるログ出力用のコールバック.
    /// メッセージを指定されたフォーマッタで文字列化し、リストに蓄積する.
    /// </summary>
    void log(const spdlog::details::log_msg& _msg) override {
        std::lock_guard<std::mutex> lock(mutex_);
        spdlog::memory_buf_t formatted;
        formatter_->format(_msg, formatted);
        logMessages_.emplace_back(fmt::to_string(formatted));
    }

    /// <summary> シンクのフラッシュ処理. ImGuiLogSink では何もしない. </summary>
    void flush() override {}

    /// <summary> ログのフォーマットパターンを設定する. </summary>
    void set_pattern(const std::string& _pattern) override {
        formatter_ = std::make_unique<spdlog::pattern_formatter>(_pattern);
    }

    /// <summary> カスタムフォーマッタを設定する. </summary>
    void set_formatter(std::unique_ptr<spdlog::formatter> _formatter) override {
        formatter_ = std::move(_formatter);
    }

    /// <summary>
    /// 蓄積されたログメッセージのリストを取得する.
    /// </summary>
    const std::vector<std::string>& GetLogMessages() const {
        return logMessages_;
    }

    /// <summary>
    /// 蓄積されたログメッセージをすべて消去する.
    /// </summary>
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        logMessages_.clear();
    }

private:
    std::vector<std::string> logMessages_; // 蓄積されたログ文字列のリスト
    std::unique_ptr<spdlog::formatter> formatter_ = std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    std::mutex mutex_; // スレッドセーフのためのミューテックス
};

} // namespace OriGine
