#pragma once

/// stl
#include <format>
#include <memory>
#include <string>

/// engine
// directX12
#include "directX12/DxDebug.h"

/// externals
#include "spdlog/spdlog.h"

namespace OriGine {
/// 前方宣言
/// engine
// directX12
class DxDebug;

/// <summary>
/// Guiにログを表示するためのクラス
/// </summary>
class GuiLogger {
public:
    GuiLogger();
    ~GuiLogger();

    void Initialize();
    void Update();
    void Finalize();

private:
    ::std::shared_ptr<spdlog::logger> logger_;
};

/// <summary>
/// ログ出力クラス
/// </summary>
class Logger {
    friend class GuiLogger;

public:
    // 初期化。アプリケーション起動時に呼ぶ。
    static void Initialize();
    static void Finalize();

private:
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectTrace(const ::std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectInfo(const ::std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectDebug(const ::std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectWarn(const ::std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectError(const ::std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectCritical(const ::std::string& message, const char* file, const char* function, int line);

public:
    /// <summary>
    /// フォーマット文字列でログ出力する関数(LogLevel : Trace)
    /// </summary>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    /// <param name="fmt"></param>
    /// <param name="...args"></param>
    template <typename... Args>
    static void Trace(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectTrace(msg, file, function, line);
    }

    /// <summary>
    /// フォーマット文字列でログ出力する関数(LogLevel : Info)
    /// </summary>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    /// <param name="fmt"></param>
    /// <param name="...args"></param>
    template <typename... Args>
    static void Info(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectInfo(msg, file, function, line);
    }
    /// <summary>
    /// フォーマット文字列でログ出力する関数(LogLevel : Debug)
    /// </summary>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    /// <param name="fmt"></param>
    /// <param name="...args"></param>
    template <typename... Args>
    static void Debug(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectDebug(msg, file, function, line);
    }
    /// <summary>
    /// フォーマット文字列でログ出力する関数(LogLevel : Warn)
    /// </summary>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    /// <param name="fmt"></param>
    /// <param name="...args"></param>
    template <typename... Args>
    static void Warn(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectWarn(msg, file, function, line);
    }
    /// <summary>
    /// フォーマット文字列でログ出力する関数(LogLevel : Error)
    /// </summary>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    /// <param name="fmt"></param>
    /// <param name="...args"></param>
    template <typename... Args>
    static void Error(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectError(msg, file, function, line);
    }
    /// <summary>
    /// フォーマット文字列でログ出力する関数(LogLevel : Critical)
    /// </summary>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    /// <param name="fmt"></param>
    /// <param name="...args"></param>
    template <typename... Args>
    static void Critical(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectCritical(msg, file, function, line);
    }

    /// <summary>
    /// DirectX関連のログを出力する関数
    /// </summary>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectXLog(const char* file, const char* function, int line);

private:
    static ::std::shared_ptr<spdlog::logger> logger_;
};

} // namespace OriGine

// マクロで簡略化
#define LOG_TRACE(fmt, ...) OriGine::Logger::Trace(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) OriGine::Logger::Info(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) OriGine::Logger::Debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) OriGine::Logger::Warn(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) OriGine::Logger::Error(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) OriGine::Logger::Critical(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_DX12() OriGine::Logger::DirectXLog(__FILE__, __FUNCTION__, __LINE__)
