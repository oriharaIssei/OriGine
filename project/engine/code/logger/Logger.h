#pragma once

/// stl
#include <format>
#include <memory>
#include <string>

///engine
// directX12
class DxDebug;

/// externals
#include "spdlog/spdlog.h"

/// <summary>
/// Guiにログを表示するためのクラス
/// </summary>
class GuiLogger{
public:
    GuiLogger();
    ~GuiLogger() ;

    void Initialize() ;
    void Update() ;
    void Finalize() ;

private:
    std::shared_ptr<spdlog::logger> logger_;
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
    static void DirectTrace(const std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectInfo(const std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectDebug(const std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectWarn(const std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectError(const std::string& message, const char* file, const char* function, int line);
    /// <summary>
    /// フォーマット済みメッセージを直接ログ出力する関数群
    /// </summary>
    /// <param name="message"></param>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectCritical(const std::string& message, const char* file, const char* function, int line);

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
    static void Trace(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectTrace(std::format(fmt, std::forward<Args>(args)...), file, function, line);
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
    static void Info(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectInfo(std::format(fmt, std::forward<Args>(args)...), file, function, line);
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
    static void Debug(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectDebug(std::format(fmt, std::forward<Args>(args)...), file, function, line);
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
    static void Warn(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectWarn(std::format(fmt, std::forward<Args>(args)...), file, function, line);
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
    static void Error(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectError(std::format(fmt, std::forward<Args>(args)...), file, function, line);
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
    static void Critical(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectCritical(std::format(fmt, std::forward<Args>(args)...), file, function, line);
    }

    /// <summary>
    /// DirectX関連のログを出力する関数
    /// </summary>
    /// <param name="file"></param>
    /// <param name="function"></param>
    /// <param name="line"></param>
    static void DirectXLog(const char* file, const char* function, int line);

private:
    static std::shared_ptr<spdlog::logger> logger_;

public:
};

// マクロで簡略化
#define LOG_TRACE(fmt, ...) Logger::Trace(__FILE__, __FUNCTION__, __LINE__,fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Logger::Info(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Logger::Debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) Logger::Warn(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::Error(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) Logger::Critical(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_DX12() Logger::DirectXLog(__FILE__, __FUNCTION__, __LINE__)
