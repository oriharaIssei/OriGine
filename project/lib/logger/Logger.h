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

    static void Trace(const std::string& message, const char* file, const char* function, int line);
    static void Info(const std::string& message, const char* file, const char* function, int line);
    static void Debug(const std::string& message, const char* file, const char* function, int line);
    static void Warn(const std::string& message, const char* file, const char* function, int line);
    static void Error(const std::string& message, const char* file, const char* function, int line);
    static void Critical(const std::string& message, const char* file, const char* function, int line);

    static void Trace(const std::wstring& message, const char* file, const char* function, int line);
    static void Info(const std::wstring& message, const char* file, const char* function, int line);
    static void Debug(const std::wstring& message, const char* file, const char* function, int line);
    static void Warn(const std::wstring& message, const char* file, const char* function, int line);
    static void Error(const std::wstring& message, const char* file, const char* function, int line);
    static void Critical(const std::wstring& message, const char* file, const char* function, int line);

    static void DirectXLog(const char* file, const char* function, int line);

private:
    static std::shared_ptr<spdlog::logger> logger_;
    static DxDebug* dxDebug_;

public:
    static void setDxDebug(DxDebug* dxDebug) {
        dxDebug_ = dxDebug;
    }
    static DxDebug* getDxDebug() {
        return dxDebug_;
    }
};

// マクロで簡略化
#define LOG_TRACE(msg) Logger::Trace(msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_INFO(msg) Logger::Info(msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_DEBUG(msg) Logger::Debug(msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_WARN(msg) Logger::Warn(msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_ERROR(msg) Logger::Error(msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_CRITICAL(msg) Logger::Critical(msg, __FILE__, __FUNCTION__, __LINE__)

#define LOG_DX12() Logger::DirectXLog(__FILE__, __FUNCTION__, __LINE__)
