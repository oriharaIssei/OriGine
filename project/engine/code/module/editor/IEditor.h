#pragma once

/// parent
#include "module/IModule.h"

/// stl
#include <concepts>

/// stl
#include <memory>
// container
#include <list>
// utility
#include <cassert>

class IEditor
    : public IModule {
public:
    IEditor()
        : IModule() {}
    virtual ~IEditor() {}

    virtual void Initialize() = 0;
    virtual void Update()     = 0;
    virtual void Finalize()   = 0;

private:
};

class IEditCommand {
public:
    IEditCommand() {}
    virtual ~IEditCommand() {}

    /// @brief コマンド実行
    virtual void Execute() = 0;

    /// @brief コマンドの取り消し
    virtual void Undo() = 0;

protected:
};

template <typename T>
concept IsEditor = std::derived_from<T, IEditor>;
