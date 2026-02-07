/**
 * @file MyGuiBase.h
 * @brief ImGuiの基本ラッパー関数（テンプレート実装）
 *
 * DragGui, SlideGui, InputGui およびそれらのVector版を提供。
 * コマンド機能なし（Undo/Redoなし）の基本版。
 */
#pragma once

#ifdef _DEBUG

/// stl
#include <string>
#include <unordered_map>

/// math
#include "math/Vector.h"

/// logger
#include "logger/Logger.h"

/// util
#include "util/nameof.h"

/// externals
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

/// <summary>
/// 値を一時的に保存するプール
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
class GuiValuePool {
public:
    GuiValuePool()  = default;
    ~GuiValuePool() = default;

private:
    std::unordered_map<std::string, T> value_;

public:
    /// <summary>
    /// 値をプールに設定
    /// </summary>
    /// <param name="_name">名前</param>
    /// <param name="_value">値</param>
    void SetValue(const std::string& _name, const T& _value) {
        // 値がすでに存在する場合は何もしない
        auto it = value_.find(_name);
        if (it != value_.end()) {
            return;
        }
        value_[_name] = _value;
    }

    /// <summary>
    /// 指定した名前の値が存在するか確認
    /// </summary>
    /// <param name="_name">名前</param>
    /// <returns>存在すればtrue</returns>
    bool hasValue(const std::string& _name) const {
        return value_.find(_name) != value_.end();
    }

    /// <summary>
    /// 値を取得し、プールから削除する
    /// </summary>
    /// <param name="_name">名前</param>
    /// <returns>取得した値</returns>
    T popValue(const std::string& _name) {
        T returnValue{};
        auto it = value_.find(_name);
        if (it != value_.end()) {
            returnValue = it->second;
            value_.erase(it);
        } else {
            LOG_ERROR("Value not found in pool: {} \n Type : {}", _name, nameof<T>());
        }
        return returnValue;
    }
};

namespace ImGui {
/// <summary>
/// std::string に対応した InputText
/// </summary>
bool InputText(const char* _label, std::string* _str, ImGuiInputTextFlags _flags = 0);
} // namespace ImGui

//==============================================================================
// DragGui - スカラー値用
//==============================================================================

/// <summary>
/// ::ImGui::Drag を型Tに対応させたラッパー
/// </summary>
template <typename T>
bool DragGui(const std::string& _label, T& _value, float _speed = 0.1f, T _min = T(), T _max = T(), const char* _format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ::ImGui::DragInt(_label.c_str(), &_value, _speed, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ::ImGui::DragFloat(_label.c_str(), &_value, _speed, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ::ImGui::DragScalar(_label.c_str(), ImGuiDataType_Double, &_value, _speed, &_min, &_max, _format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return ::ImGui::DragScalar(_label.c_str(), ImGuiDataType_U32, &_value, _speed, &_min, &_max, _format);
    } else {
        return false;
    }
}

//==============================================================================
// DragVectorGui - N次元ベクトル用
//==============================================================================

/// <summary>
/// ::ImGui::Drag を型TのN次元ベクトルに対応させたラッパー
/// </summary>
template <int N, typename T>
bool DragVectorGui(const std::string& _label, OriGine::Vector<N, T>& _value, float _speed = 0.1f, T _min = T(), T _max = T(), const char* _format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ::ImGui::DragInt2(_label.c_str(), _value.v, _speed, _min, _max, _format);
        if constexpr (N == 3)
            return ::ImGui::DragInt3(_label.c_str(), _value.v, _speed, _min, _max, _format);
        if constexpr (N == 4)
            return ::ImGui::DragInt4(_label.c_str(), _value.v, _speed, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ::ImGui::DragFloat2(_label.c_str(), _value.v, _speed, _min, _max, _format);
        if constexpr (N == 3)
            return ::ImGui::DragFloat3(_label.c_str(), _value.v, _speed, _min, _max, _format);
        if constexpr (N == 4)
            return ::ImGui::DragFloat4(_label.c_str(), _value.v, _speed, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ::ImGui::DragScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 2, _speed, &_min, &_max, _format);
        if constexpr (N == 3)
            return ::ImGui::DragScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 3, _speed, &_min, &_max, _format);
        if constexpr (N == 4)
            return ::ImGui::DragScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 4, _speed, &_min, &_max, _format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        if constexpr (N == 2)
            return ::ImGui::DragScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 2, _speed, &_min, &_max, _format);
        if constexpr (N == 3)
            return ::ImGui::DragScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 3, _speed, &_min, &_max, _format);
        if constexpr (N == 4)
            return ::ImGui::DragScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 4, _speed, &_min, &_max, _format);
    } else {
        return false;
    }
}

//==============================================================================
// SlideGui - スカラー値用
//==============================================================================

/// <summary>
/// ::ImGui::Slider を型Tに対応させたラッパー
/// </summary>
template <typename T>
bool SlideGui(const std::string& _label, T& _value, T _min = T(), T _max = T(), const char* _format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ::ImGui::SliderInt(_label.c_str(), &_value, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ::ImGui::SliderFloat(_label.c_str(), &_value, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ::ImGui::SliderScalar(_label.c_str(), ImGuiDataType_Double, &_value, &_min, &_max, _format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return ::ImGui::SliderScalar(_label.c_str(), ImGuiDataType_U32, &_value, &_min, &_max, _format);
    } else {
        return false;
    }
}

//==============================================================================
// SlideVectorGui - N次元ベクトル用
//==============================================================================

/// <summary>
/// ::ImGui::Slider を型TのN次元ベクトルに対応させたラッパー
/// </summary>
template <typename T, int N>
bool SlideVectorGui(const std::string& _label, OriGine::Vector<N, T>& _value, T _min = T(), T _max = T(), const char* _format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ::ImGui::SliderInt2(_label.c_str(), _value.v, _min, _max, _format);
        if constexpr (N == 3)
            return ::ImGui::SliderInt3(_label.c_str(), _value.v, _min, _max, _format);
        if constexpr (N == 4)
            return ::ImGui::SliderInt4(_label.c_str(), _value.v, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ::ImGui::SliderFloat2(_label.c_str(), _value.v, _min, _max, _format);
        if constexpr (N == 3)
            return ::ImGui::SliderFloat3(_label.c_str(), _value.v, _min, _max, _format);
        if constexpr (N == 4)
            return ::ImGui::SliderFloat4(_label.c_str(), _value.v, _min, _max, _format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ::ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 2, &_min, &_max, _format);
        if constexpr (N == 3)
            return ::ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 3, &_min, &_max, _format);
        if constexpr (N == 4)
            return ::ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 4, &_min, &_max, _format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        if constexpr (N == 2)
            return ::ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 2, &_min, &_max, _format);
        if constexpr (N == 3)
            return ::ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 3, &_min, &_max, _format);
        if constexpr (N == 4)
            return ::ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 4, &_min, &_max, _format);
    } else {
        return false;
    }
}

//==============================================================================
// InputGui - スカラー値用
//==============================================================================

/// <summary>
/// ::ImGui::Input を型Tに対応させたラッパー
/// </summary>
template <typename T>
bool InputGui(const std::string& _label, T& _value, const char* _format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ::ImGui::InputInt(_label.c_str(), &_value);
    } else if constexpr (std::is_same_v<T, float>) {
        return ::ImGui::InputFloat(_label.c_str(), &_value, 0.0f, 0.0f, _format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ::ImGui::InputScalar(_label.c_str(), ImGuiDataType_Double, &_value, nullptr, nullptr, _format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return ::ImGui::InputScalar(_label.c_str(), ImGuiDataType_U32, &_value, nullptr, nullptr, _format);
    } else {
        return false;
    }
}

//==============================================================================
// InputVectorGui - N次元ベクトル用
//==============================================================================

/// <summary>
/// ::ImGui::Input を型TのN次元ベクトルに対応させたラッパー
/// </summary>
template <typename T, int N>
bool InputVectorGui(const std::string& _label, OriGine::Vector<N, T>& _value, const char* _format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ::ImGui::InputInt2(_label.c_str(), _value.v);
        if constexpr (N == 3)
            return ::ImGui::InputInt3(_label.c_str(), _value.v);
        if constexpr (N == 4)
            return ::ImGui::InputInt4(_label.c_str(), _value.v);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ::ImGui::InputFloat2(_label.c_str(), _value.v, _format);
        if constexpr (N == 3)
            return ::ImGui::InputFloat3(_label.c_str(), _value.v, _format);
        if constexpr (N == 4)
            return ::ImGui::InputFloat4(_label.c_str(), _value.v, _format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ::ImGui::InputScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 2, nullptr, nullptr, _format);
        if constexpr (N == 3)
            return ::ImGui::InputScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 3, nullptr, nullptr, _format);
        if constexpr (N == 4)
            return ::ImGui::InputScalarN(_label.c_str(), ImGuiDataType_Double, _value.v, 4, nullptr, nullptr, _format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        if constexpr (N == 2)
            return ::ImGui::InputScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 2, nullptr, nullptr, _format);
        if constexpr (N == 3)
            return ::ImGui::InputScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 3, nullptr, nullptr, _format);
        if constexpr (N == 4)
            return ::ImGui::InputScalarN(_label.c_str(), ImGuiDataType_U32, _value.v, 4, nullptr, nullptr, _format);
    } else {
        return false;
    }
}

//==============================================================================
// ColorEditGui - 3または4次元ベクトル用
//==============================================================================

/// <summary>
/// ::ImGui::ColorEdit を型Nの3または4次元ベクトルに対応させたラッパー
/// </summary>
template <int N>
bool ColorEditGui(const std::string& _label, OriGine::Vector<N, float>& _value, ImGuiColorEditFlags _colorEditFlags = 0) {
    static_assert(N == 3 || N == 4, "ColorEditGui only supports 3 or 4 components (RGB or RGBA).");

    if constexpr (N == 3) {
        return ::ImGui::ColorEdit3(_label.c_str(), _value.v, _colorEditFlags);
    } else if constexpr (N == 4) {
        return ::ImGui::ColorEdit4(_label.c_str(), _value.v, _colorEditFlags);
    } else {
        return false;
    }
}

#endif // _DEBUG
