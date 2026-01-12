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
/// ImGui 上にログを表示するための管轄クラス.
/// spdlog の Sink として動作し、毎レジストリフレームの更新を行う.
/// </summary>
class GuiLogger {
public:
    GuiLogger();
    ~GuiLogger();

    /// <summary>
    /// GuiLogger の初期化。spdlog インスタンスの生成などを行う.
    /// </summary>
    void Initialize();

    /// <summary>
    /// ログウィンドウの表示更新を行う.
    /// </summary>
    void Update();

    /// <summary>
    /// 終了時のリソース解放.
    /// </summary>
    void Finalize();

private:
    ::std::shared_ptr<spdlog::logger> logger_; // spdlog のロガーインスタンス
};

/// <summary>
/// エンジン全体のロギングシステムを管理する静的クラス.
/// コンソール出力、ファイル出力、および GuiLogger への出力を統括する.
/// </summary>
class Logger {
    friend class GuiLogger;

public:
    /// <summary>
    /// ロギングシステムの構築. アプリケーション起動時に一度だけ呼び出す.
    /// </summary>
    static void Initialize();

    /// <summary>
    /// ロギングシステムのシャットダウン処理.
    /// </summary>
    static void Finalize();

private:
    /// <summary>
    /// TRACE レベルのログを直接書き込む. (通常はマクロ経由で使用)
    /// </summary>
    static void DirectTrace(const ::std::string& message, const char* file, const char* function, int line);

    /// <summary>
    /// INFO レベルのログを直接書き込む.
    /// </summary>
    static void DirectInfo(const ::std::string& message, const char* file, const char* function, int line);

    /// <summary>
    /// DEBUG レベルのログを直接書き込む.
    /// </summary>
    static void DirectDebug(const ::std::string& message, const char* file, const char* function, int line);

    /// <summary>
    /// WARN レベルのログを直接書き込む.
    /// </summary>
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
    /// std::format 形式の文字列を受け取り、TRACE レベルでログを出力する.
    /// ファイル名、関数名、行番号を自動的に付加する.
    /// </summary>
    template <typename... Args>
    static void Trace(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectTrace(msg, file, function, line);
    }

    /// <summary>
    /// std::format 形式の文字列を受け取り、INFO レベルでログを出力する.
    /// </summary>
    template <typename... Args>
    static void Info(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectInfo(msg, file, function, line);
    }

    /// <summary>
    /// std::format 形式の文字列を受け取り、DEBUG レベルでログを出力する.
    /// </summary>
    template <typename... Args>
    static void Debug(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectDebug(msg, file, function, line);
    }

    /// <summary>
    /// std::format 形式の文字列を受け取り、WARN レベルでログを出力する.
    /// </summary>
    template <typename... Args>
    static void Warn(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectWarn(msg, file, function, line);
    }

    /// <summary>
    /// std::format 形式の文字列を受け取り、ERROR レベルでログを出力する.
    /// </summary>
    template <typename... Args>
    static void Error(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectError(msg, file, function, line);
    }

    /// <summary>
    /// std::format 形式の文字列を受け取り、CRITICAL レベルでログを出力する.
    /// エンジンの中断を伴うような致命的なエラー時に使用する.
    /// </summary>
    template <typename... Args>
    static void Critical(const char* file, const char* function, int line, std::string_view fmt, Args&&... args) {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        DirectCritical(msg, file, function, line);
    }

    /// <summary>
    /// DirectX12 のデバッグレイヤーが保持しているメッセージを取得し、ログとして出力する.
    /// </summary>
    static void DirectXLog(const char* file, const char* function, int line);

private:
    static ::std::shared_ptr<spdlog::logger> logger_; // コアとなる spdlog ロガー
};

} // namespace OriGine

// マクロで簡略化

/// <summary> TRACE レベルのログを出力するマクロ. </summary>
#define LOG_TRACE(fmt, ...) OriGine::Logger::Trace(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/// <summary> INFO レベルのログを出力するマクロ. </summary>
#define LOG_INFO(fmt, ...) OriGine::Logger::Info(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/// <summary> DEBUG レベルのログを出力するマクロ. </summary>
#define LOG_DEBUG(fmt, ...) OriGine::Logger::Debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/// <summary> WARN レベルのログを出力するマクロ. </summary>
#define LOG_WARN(fmt, ...) OriGine::Logger::Warn(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/// <summary> ERROR レベルのログを出力するマクロ. </summary>
#define LOG_ERROR(fmt, ...) OriGine::Logger::Error(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/// <summary> CRITICAL レベルのログを出力するマクロ. </summary>
#define LOG_CRITICAL(fmt, ...) OriGine::Logger::Critical(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/// <summary> DirectX12 のデバッグログを出力するマクロ. </summary>
#define LOG_DX12() OriGine::Logger::DirectXLog(__FILE__, __FUNCTION__, __LINE__)
