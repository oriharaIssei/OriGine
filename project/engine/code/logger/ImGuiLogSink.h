#pragma once

/// stl
#include <memory>
#include <mutex>
#include <string>

#include <vector>

#include <spdlog/sinks/sink.h>
#include <spdlog/spdlog.h>

/// <summary>
/// ImGui表示用のログシンク
/// </summary>
class ImGuiLogSink : public spdlog::sinks::sink {
public:
    ImGuiLogSink()           = default;
    ~ImGuiLogSink() override = default;

    void log(const spdlog::details::log_msg& msg) override {
        std::lock_guard<std::mutex> lock(mutex_);
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        logMessages_.emplace_back(fmt::to_string(formatted));
    }

    void flush() override {}

    void set_pattern(const std::string& pattern) override {
        formatter_ = std::make_unique<spdlog::pattern_formatter>(pattern);
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override {
        formatter_ = std::move(sink_formatter);
    }

    const std::vector<std::string>& GetLogMessages() const {
        return logMessages_;
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        logMessages_.clear();
    }

private:
    std::vector<std::string> logMessages_;
    std::unique_ptr<spdlog::formatter> formatter_ = std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    std::mutex mutex_;
};
