#pragma once

/// parent
#include "module/IModule.h"

/// stl
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <vector>

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
template <typename T>
concept IsEditor = std::derived_from<T, IEditor>;

#pragma region DefaultCommand
class IEditCommand {
public:
    IEditCommand() {}
    virtual ~IEditCommand() {}

    /// @brief コマンド実行
    virtual void Execute() = 0;

    /// @brief コマンドの取り消し
    virtual void Undo() = 0;
};

class CommandCombo
    : public IEditCommand {
public:
    CommandCombo() {}
    ~CommandCombo() {}

    void Execute() {
        for (auto& command : commands_) {
            command->Execute();
        }
        if (funcOnAfterCommand_) {
            funcOnAfterCommand_();
        }
    }
    void Undo() {
        for (auto& command : commands_) {
            command->Undo();
        }
        if (funcOnAfterCommand_) {
            funcOnAfterCommand_();
        }
    }

private:
    std::vector<std::shared_ptr<IEditCommand>> commands_;
    std::function<void()> funcOnAfterCommand_;
    std::function<void()> funcOnAfterUndoCommand_;

public:
    void addCommand(std::shared_ptr<IEditCommand> command) {
        commands_.emplace_back(command);
    }
    void setFuncOnAfterCommand(std::function<void()> func, bool _syncFuncOnAfterUndoCommand = false) {
        funcOnAfterCommand_ = func;
        if (_syncFuncOnAfterUndoCommand) {
            funcOnAfterUndoCommand_ = func;
        }
    }
    void setFuncOnAfterUndoCommand(std::function<void()> func) {
        funcOnAfterUndoCommand_ = func;
    }
};

template <typename T>
class SetterCommand
    : public IEditCommand {
public:
    SetterCommand(T* _target, const T& _value, std::function<void(T*)> _funcOnAfterExecute = nullptr, bool _syncFuncOnAfter = true, std::function<void(T*)> _funcOnAfterUndo = nullptr)
        : target(_target), value(_value), funcOnAfterExecute_(_funcOnAfterExecute) {
        if (_syncFuncOnAfter) {
            funcOnAfterUndoExecute_ = _funcOnAfterExecute;
        } else {
            funcOnAfterUndoExecute_ = _funcOnAfterUndo;
        }
    }
    void Execute() override {
        if (!target) {
            return;
        }
        oldValue = *target;
        *target  = value;

        if (funcOnAfterExecute_) {
            funcOnAfterExecute_(target);
        }
    }
    void Undo() override {
        if (!target) {
            return;
        }
        *target = oldValue;
        if (funcOnAfterUndoExecute_) {
            funcOnAfterUndoExecute_(target);
        }
    }

private:
    std::function<void(T*)> funcOnAfterExecute_;
    std::function<void(T*)> funcOnAfterUndoExecute_;
    T* target;
    T value;
    T oldValue;
};

template <typename T>
concept HasPushBack = requires(T a, typename T::value_type v) {
    { a.push_back(v) };
};
template <typename T>
concept HasPushFront = requires(T a, typename T::value_type v) {
    { a.push_front(v) };
};

template <typename T>
concept HasClear = requires(T a) {
    { a.clear() };
};
template <typename T>
concept HasErase = requires(T a, typename T::iterator it) {
    { a.erase(it) };
};

template <typename Container>
    requires HasPushBack<Container>
class AddElementCommand : public IEditCommand {
public:
    using ContainerType = Container;
    using Iterator      = typename Container::iterator;
    using ValueType     = typename Container::value_type;

    AddElementCommand(Container* container, const ValueType& value)
        : container_(container), value_(value) {}

    void Execute() override {
        container_->push_back(value_);
    }

    void Undo() override {
        if (!container_->empty()) {
            container_->pop_back();
        }
    }

private:
    Container* container_;
    ValueType value_;
};

// erase対応コンテナ用コマンド
template <typename Container>
    requires HasErase<Container>
class EraseElementCommand : public IEditCommand {
public:
    using ContainerType = Container;
    using Iterator      = typename Container::iterator;

    EraseElementCommand(Container* container, Iterator it)
        : container_(container), it_(it) {}

    void Execute() override {
        if (container_ && it_ != container_->end()) {
            backup_ = *it_;
            it_     = container_->erase(it_);
        }
    }

    void Undo() override {
        it_ = container_->insert(it_, backup_);
    }

private:
    Container* container_;
    Iterator it_;
    typename Container::value_type backup_;
};

template <typename Container>
class SortCommand : public IEditCommand {
public:
    using ValueType = typename Container::value_type;
    using Compare   = std::function<bool(const ValueType&, const ValueType&)>;

    SortCommand(Container* container, Compare comp)
        : container_(container), comp_(comp) {}

    void Execute() override {
        // 元の順序を保存
        backup_.assign(container_->begin(), container_->end());
        std::sort(container_->begin(), container_->end(), comp_);
    }

    void Undo() override {
        // 元の順序に戻す
        if (container_) {
            container_->assign(backup_.begin(), backup_.end());
        }
    }

private:
    Container* container_;
    Compare comp_;
    std::vector<ValueType> backup_;
};

template <typename Container>
    requires HasClear<Container>
class ClearCommand : public IEditCommand {
public:
    using ValueType = typename Container::value_type;

    ClearCommand(Container* container)
        : container_(container) {}
    void Execute() override {
        // 元の順序を保存
        backup_.assign(container_->begin(), container_->end());
        container_->clear();
    }
    void Undo() override {
        // 元の順序に戻す
        if (container_) {
            container_->assign(backup_.begin(), backup_.end());
        }
    }

private:
    Container* container_;
    std::vector<typename Container::value_type> backup_;
};

#pragma endregion DefaultCommand
