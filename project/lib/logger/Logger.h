#pragma once

/// stl
#include <format>
#include <memory>
#include <string>

/// externals
#include "spdlog/spdlog.h"

class Logger {
public:
    // 初期化。アプリケーション起動時に呼ぶ。
    static void Initialize();

    static void Trace(const std::string& message);
    static void Info(const std::string& message);
    static void Debug(const std::string& message);
    static void Warn(const std::string& message);
    static void Error(const std::string& message);
    static void Critical(const std::string& message);

    static void Trace(const std::wstring& message);
    static void Info(const std::wstring& message);
    static void Debug(const std::wstring& message);
    static void Warn(const std::wstring& message);
    static void Error(const std::wstring& message);
    static void Critical(const std::wstring& message);

private:
    static std::shared_ptr<spdlog::logger> logger_;
};
