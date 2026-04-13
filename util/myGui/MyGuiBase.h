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
    /// <param name="name">名前</param>
    /// <param name="value">値</param>
    void SetValue(const std::string& name, const T& value) {
        // 値がすでに存在する場合は何もしない
        auto it = value_.find(name);
        if (it != value_.end()) {
            return;
        }
        value_[name] = value;
    }

    /// <summary>
    /// 指定した名前の値が存在するか確認
    /// </summary>
    /// <param name="name">名前</param>
    /// <returns>存在すればtrue</returns>
    bool hasValue(const std::string& name) const {
        return value_.find(name) != value_.end();
    }

    /// <summary>
    /// 値を取得し、プールから削除する
    /// </summary>
    /// <param name="name">名前</param>
    /// <returns>取得した値</returns>
    T popValue(const std::string& name) {
        T returnValue{};
        auto it = value_.find(name);
        if (it != value_.end()) {
            returnValue = it->second;
            value_.erase(it);
        } else {
            LOG_ERROR("Value not found in pool: {} \n Type : {}", name, nameof<T>());
        }
        return returnValue;
    }
};

namespace ImGui {
/// <summary>
/// std::string に対応した InputText
/// </summary>
bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0);
} // namespace ImGui

//==============================================================================
// DragGui - スカラー値用
//==============================================================================

/// <summary>
/// ::ImGui::Drag を型Tに対応させたラッパー
/// </summary>
template <typename T>
bool DragGui(const std::string& label, T& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ::ImGui::DragInt(label.c_str(), &value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ::ImGui::DragFloat(label.c_str(), &value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ::ImGui::DragScalar(label.c_str(), ImGuiDataType_Double, &value, speed, &min, &max, format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return ::ImGui::DragScalar(label.c_str(), ImGuiDataType_U32, &value, speed, &min, &max, format);
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
bool DragVectorGui(const std::string& label, OriGine::Vector<N, T>& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ::ImGui::DragInt2(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 3)
            return ::ImGui::DragInt3(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 4)
            return ::ImGui::DragInt4(label.c_str(), value.v, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ::ImGui::DragFloat2(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 3)
            return ::ImGui::DragFloat3(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 4)
            return ::ImGui::DragFloat4(label.c_str(), value.v, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ::ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value.v, 2, speed, &min, &max, format);
        if constexpr (N == 3)
            return ::ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value.v, 3, speed, &min, &max, format);
        if constexpr (N == 4)
            return ::ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value.v, 4, speed, &min, &max, format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        if constexpr (N == 2)
            return ::ImGui::DragScalarN(label.c_str(), ImGuiDataType_U32, value.v, 2, speed, &min, &max, format);
        if constexpr (N == 3)
            return ::ImGui::DragScalarN(label.c_str(), ImGuiDataType_U32, value.v, 3, speed, &min, &max, format);
        if constexpr (N == 4)
            return ::ImGui::DragScalarN(label.c_str(), ImGuiDataType_U32, value.v, 4, speed, &min, &max, format);
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
bool SlideGui(const std::string& label, T& value, T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ::ImGui::SliderInt(label.c_str(), &value, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ::ImGui::SliderFloat(label.c_str(), &value, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ::ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, &value, &min, &max, format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return ::ImGui::SliderScalar(label.c_str(), ImGuiDataType_U32, &value, &min, &max, format);
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
bool SlideVectorGui(const std::string& label, OriGine::Vector<N, T>& value, T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ::ImGui::SliderInt2(label.c_str(), value.v, min, max, format);
        if constexpr (N == 3)
            return ::ImGui::SliderInt3(label.c_str(), value.v, min, max, format);
        if constexpr (N == 4)
            return ::ImGui::SliderInt4(label.c_str(), value.v, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ::ImGui::SliderFloat2(label.c_str(), value.v, min, max, format);
        if constexpr (N == 3)
            return ::ImGui::SliderFloat3(label.c_str(), value.v, min, max, format);
        if constexpr (N == 4)
            return ::ImGui::SliderFloat4(label.c_str(), value.v, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ::ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value.v, 2, &min, &max, format);
        if constexpr (N == 3)
            return ::ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value.v, 3, &min, &max, format);
        if constexpr (N == 4)
            return ::ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value.v, 4, &min, &max, format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        if constexpr (N == 2)
            return ::ImGui::SliderScalarN(label.c_str(), ImGuiDataType_U32, value.v, 2, &min, &max, format);
        if constexpr (N == 3)
            return ::ImGui::SliderScalarN(label.c_str(), ImGuiDataType_U32, value.v, 3, &min, &max, format);
        if constexpr (N == 4)
            return ::ImGui::SliderScalarN(label.c_str(), ImGuiDataType_U32, value.v, 4, &min, &max, format);
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
bool InputGui(const std::string& label, T& value, const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ::ImGui::InputInt(label.c_str(), &value);
    } else if constexpr (std::is_same_v<T, float>) {
        return ::ImGui::InputFloat(label.c_str(), &value, 0.0f, 0.0f, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ::ImGui::InputScalar(label.c_str(), ImGuiDataType_Double, &value, nullptr, nullptr, format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return ::ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &value, nullptr, nullptr, format);
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
bool InputVectorGui(const std::string& label, OriGine::Vector<N, T>& value, const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ::ImGui::InputInt2(label.c_str(), value.v);
        if constexpr (N == 3)
            return ::ImGui::InputInt3(label.c_str(), value.v);
        if constexpr (N == 4)
            return ::ImGui::InputInt4(label.c_str(), value.v);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ::ImGui::InputFloat2(label.c_str(), value.v, format);
        if constexpr (N == 3)
            return ::ImGui::InputFloat3(label.c_str(), value.v, format);
        if constexpr (N == 4)
            return ::ImGui::InputFloat4(label.c_str(), value.v, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ::ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value.v, 2, nullptr, nullptr, format);
        if constexpr (N == 3)
            return ::ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value.v, 3, nullptr, nullptr, format);
        if constexpr (N == 4)
            return ::ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value.v, 4, nullptr, nullptr, format);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        if constexpr (N == 2)
            return ::ImGui::InputScalarN(label.c_str(), ImGuiDataType_U32, value.v, 2, nullptr, nullptr, format);
        if constexpr (N == 3)
            return ::ImGui::InputScalarN(label.c_str(), ImGuiDataType_U32, value.v, 3, nullptr, nullptr, format);
        if constexpr (N == 4)
            return ::ImGui::InputScalarN(label.c_str(), ImGuiDataType_U32, value.v, 4, nullptr, nullptr, format);
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
bool ColorEditGui(const std::string& label, OriGine::Vector<N, float>& value, ImGuiColorEditFlags colorEditFlags = 0) {
    static_assert(N == 3 || N == 4, "ColorEditGui only supports 3 or 4 components (RGB or RGBA).");

    if constexpr (N == 3) {
        return ::ImGui::ColorEdit3(label.c_str(), value.v, colorEditFlags);
    } else if constexpr (N == 4) {
        return ::ImGui::ColorEdit4(label.c_str(), value.v, colorEditFlags);
    } else {
        return false;
    }
}

#endif // _DEBUG
