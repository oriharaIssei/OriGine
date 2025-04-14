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

class Logger {
    friend class GuiLogger;

public:
    // 初期化。アプリケーション起動時に呼ぶ。
    static void
    Initialize();
    static void Finalize();

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
