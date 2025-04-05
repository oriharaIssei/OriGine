#pragma once

/// parent
#include "module/IModule.h"

/// stl
#include <concepts>

class IDebugger
    : public IModule {
public:
    IDebugger()
        : IModule() {}
    virtual ~IDebugger() {}

    virtual void Initialize() = 0;
    virtual void Update()     = 0;
    virtual void Finalize()   = 0;

private:
};

template <typename T>
concept IsDebuggerGroup = std::derived_from<T, IDebugger>;

class IDebugCommand {
public:
    IDebugCommand() {}
    virtual ~IDebugCommand() {}

    /// @brief コマンド実行
    virtual void Execute() = 0;

    /// @brief コマンドの取り消し
    virtual void Undo() = 0;

protected:
};
