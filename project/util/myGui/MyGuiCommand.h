/**
 * @file MyGuiCommand.h
 * @brief ImGuiのコマンド対応ラッパー関数（テンプレート実装）
 *
 * Undo/Redo機能を提供するコマンド付きGUIラッパー。
 * DragGuiCommand, SlideGuiCommand, InputGuiCommand, ColorEditGuiCommand等。
 */
#pragma once

#ifdef _DEBUG

/// stl
#include <functional>
#include <memory>
#include <string>

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"

/// myGui
#include "MyGuiBase.h"

//==============================================================================
// DragGuiCommand - スカラー値用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Drag を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T>
bool DragGuiCommand(const std::string& _label, T& _value, float _speed = 0.1f, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = DragGui(_label, _value, _speed, _min, _max, _format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(_label, _value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = _value;
        _value     = valuePool.popValue(_label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<T>>(&_value, newValue, _afterFunc));
        result = true;
    }

    return result;
}

//==============================================================================
// DragGuiVectorCommand - N次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Drag を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <int N, typename T>
bool DragGuiVectorCommand(const std::string& _label, OriGine::Vector<N, T>& _value, float _speed = 0.1f, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = DragVectorGui(_label, _value, _speed, _min, _max, _format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(_label, _value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, T> newValue = _value;
        _value                         = valuePool.popValue(_label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, T>>>(&_value, newValue, _afterFunc));
        result = true;
    }

    return result;
}

//==============================================================================
// SlideGuiCommand - スカラー値用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Slider を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T>
bool SlideGuiCommand(const std::string& _label, T& _value, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = SlideGui(_label, _value, _min, _max, _format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(_label, _value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = _value;
        _value     = valuePool.popValue(_label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<T>>(&_value, newValue, _afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// SlideVectorCommand - N次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Slider を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T, int N>
bool SlideVectorCommand(const std::string& _label, OriGine::Vector<N, T>& _value, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = SlideVectorGui(_label, _value, _min, _max, _format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(_label, _value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, T> newValue = _value;
        _value                         = valuePool.popValue(_label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, T>>>(&_value, newValue, _afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// InputGuiCommand - スカラー値用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Input を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T>
bool InputGuiCommand(const std::string& _label, T& _value, const char* _format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = InputGui(_label, _value, _format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(_label, _value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = _value;
        _value     = valuePool.popValue(_label);
        ::OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<T>>(&_value, newValue, _afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// InputVectorGuiCommand - N次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Input を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T, int N>
bool InputVectorGuiCommand(const std::string& _label, OriGine::Vector<N, T>& _value, const char* _format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = InputVectorGui(_label, _value, _format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(_label, _value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, T> newValue = _value;
        _value                         = valuePool.popValue(_label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, T>>>(&_value, newValue, _afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// ColorEditGuiCommand - 3または4次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::ColorEdit を型Nの3または4次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <int N>
bool ColorEditGuiCommand(const std::string& _label, OriGine::Vector<N, float>& _value, ImGuiColorEditFlags _colorEditFlags = 0, std::function<void(OriGine::Vector<N, float>*)> _afterFunc = nullptr) {
    static_assert(N == 3 || N == 4, "ColorEditGuiCommand only supports 3 or 4 components (RGB or RGBA).");

    static GuiValuePool<OriGine::Vector<N, float>> valuePool;

    bool result = ColorEditGui<N>(_label, _value, _colorEditFlags);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(_label, _value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, float> newValue = _value;
        _value                             = valuePool.popValue(_label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, float>>>(&_value, newValue, _afterFunc));
        return true;
    }

    return result;
}

#endif // _DEBUG
