#pragma once

/// stl
#include <format>
#include <memory>
#include <string>

#include "module/debugger/IDebugger.h"

/// externals
#include "spdlog/spdlog.h"

class GuiLogger
    : public IDebugger {
public:
    GuiLogger();
    ~GuiLogger() override;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

private:
    std::shared_ptr<spdlog::logger> logger_;
};

class DxDebug;
class Logger {
    friend class GuiLogger;

public:
    // 初期化。アプリケーション起動時に呼ぶ。
    static void Initialize();
    static void Finalize();

private:
    static void DirectTrace(const std::string& message, const char* file, const char* function, int line);
    static void DirectInfo(const std::string& message, const char* file, const char* function, int line);
    static void DirectDebug(const std::string& message, const char* file, const char* function, int line);
    static void DirectWarn(const std::string& message, const char* file, const char* function, int line);
    static void DirectError(const std::string& message, const char* file, const char* function, int line);
    static void DirectCritical(const std::string& message, const char* file, const char* function, int line);

public:
    template <typename... Args>
    static void Trace(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectTrace(std::format(fmt, std::forward<Args>(args)...), file, function, line);
    }
    template <typename... Args>
    static void Info(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectInfo(std::format(fmt, std::forward<Args>(args)...), file, function, line);
    }
    template <typename... Args>
    static void Debug(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectDebug(std::format(fmt, std::forward<Args>(args)...), file, function, line);
    }
    template <typename... Args>
    static void Warn(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectWarn(std::format(fmt, std::forward<Args>(args)...), file, function, line);
    }
    template <typename... Args>
    static void Error(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectError(std::format(fmt, std::forward<Args>(args)...), file, function, line);
    }
    template <typename... Args>
    static void Critical(const char* file, const char* function, int line, std::format_string<Args...> fmt, Args&&... args) {
        DirectCritical(std::format(fmt, std::forward<Args>(args)...), file, function, line);
    }

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
