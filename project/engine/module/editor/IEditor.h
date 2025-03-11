#pragma once

/// parent
#include "module/IModule.h"

/// stl
#include <memory>
// container
#include <list>
// utility
#include <cassert>
#include <concepts>

class IEditor
    : public IModule {
public:
    IEditor()
        : IModule() {}
    virtual ~IEditor() {}

    virtual void Update() = 0;

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
