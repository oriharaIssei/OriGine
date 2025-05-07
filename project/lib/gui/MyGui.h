#pragma once

#ifdef _DEBUG
/// stl
#include <memory>
#include <string>

/// engine
#include "module/editor/EditorGroup.h"
#include "module/editor/IEditor.h"

/// externals
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

template <typename T>
bool DragGui(const std::string& label, T& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ImGui::DragInt(label.c_str(), &value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ImGui::DragFloat(label.c_str(), &value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ImGui::DragScalar(label.c_str(), ImGuiDataType_Double, &value, speed, &min, &max, format);
    }
    return false; // サポートされていない型の場合
}

template <typename T, int N>
bool DragGui(const std::string& label, T (&value)[N], float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ImGui::DragInt2(label.c_str(), value, speed, min, max, format);
        if constexpr (N == 3)
            return ImGui::DragInt3(label.c_str(), value, speed, min, max, format);
        if constexpr (N == 4)
            return ImGui::DragInt4(label.c_str(), value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ImGui::DragFloat2(label.c_str(), value, speed, min, max, format);
        if constexpr (N == 3)
            return ImGui::DragFloat3(label.c_str(), value, speed, min, max, format);
        if constexpr (N == 4)
            return ImGui::DragFloat4(label.c_str(), value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value, 2, speed, &min, &max, format);
        if constexpr (N == 3)
            return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value, 3, speed, &min, &max, format);
        if constexpr (N == 4)
            return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value, 4, speed, &min, &max, format);
    }
    return false; // サポートされていない型の場合
}

template <typename T>
bool SlideGui(const std::string& label, T& value, T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ImGui::SliderInt(label.c_str(), &value, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ImGui::SliderFloat(label.c_str(), &value, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, &value, &min, &max, format);
    }
    return false; // サポートされていない型の場合
}
template <typename T, int N>
bool SlideGui(const std::string& label, T (&value)[N], T min = T(), T max = T(), const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ImGui::SliderInt2(label.c_str(), value, min, max, format);
        if constexpr (N == 3)
            return ImGui::SliderInt3(label.c_str(), value, min, max, format);
        if constexpr (N == 4)
            return ImGui::SliderInt4(label.c_str(), value, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ImGui::SliderFloat2(label.c_str(), value, min, max, format);
        if constexpr (N == 3)
            return ImGui::SliderFloat3(label.c_str(), value, min, max, format);
        if constexpr (N == 4)
            return ImGui::SliderFloat4(label.c_str(), value, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value, 2, &min, &max, format);
        if constexpr (N == 3)
            return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value, 3, &min, &max, format);
        if constexpr (N == 4)
            return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value, 4, &min, &max, format);
    }
    return false; // サポートされていない型の場合
}

template <typename T>
bool InputGui(const std::string& label, T& value, const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        return ImGui::InputInt(label.c_str(), &value);
    } else if constexpr (std::is_same_v<T, float>) {
        return ImGui::InputFloat(label.c_str(), &value, 0.0f, 0.0f, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ImGui::InputScalar(label.c_str(), ImGuiDataType_Double, &value, nullptr, nullptr, format);
    }
    return false; // サポートされていない型の場合
}
template <typename T, int N>
bool InputGui(const std::string& label, T (&value)[N], const char* format = "%.3f") {
    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ImGui::InputInt2(label.c_str(), value);
        if constexpr (N == 3)
            return ImGui::InputInt3(label.c_str(), value);
        if constexpr (N == 4)
            return ImGui::InputInt4(label.c_str(), value);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ImGui::InputFloat2(label.c_str(), value, format);
        if constexpr (N == 3)
            return ImGui::InputFloat3(label.c_str(), value, format);
        if constexpr (N == 4)
            return ImGui::InputFloat4(label.c_str(), value, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value, 2, nullptr, nullptr, format);
        if constexpr (N == 3)
            return ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value, 3, nullptr, nullptr, format);
        if constexpr (N == 4)
            return ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value, 4, nullptr, nullptr, format);
    }
    return false; // サポートされていない型の場合
}

template <typename T>
bool DragCommand(const std::string& label, T& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {
    T newValue = value; // 現在の値をコピー
    if (DragGui(label, newValue, speed, min, max, format)) { // 値が変更された場合
        EditorGroup::getInstance()->pushCommand(std::make_unique<SetComamnd<T>>(value, &newValue));
        return true;
    }
    return false;
}
template <typename T, int N>
bool DragCommand(const std::string& label, T (&value)[N], float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {
    T newValue[N]; // 現在の値をコピー
    std::copy(value, value + N, newValue);
    if (DragGui(label, newValue, speed, min, max, format)) { // 値が変更された場合
        EditorGroup::getInstance()->pushCommand(std::make_unique<SetComamnd<T[N]>>(value, &newValue));
        return true;
    }
    return false;
}

template <typename T>
bool SlideCommand(const std::string& label, T& value, T min = T(), T max = T(), const char* format = "%.3f") {
    T newValue = value; // 現在の値をコピー
    if (SlideGui(label, newValue, min, max, format)) { // 値が変更された場合
        EditorGroup::getInstance()->pushCommand(std::make_unique<SetComamnd<T>>(value, &newValue));
        return true;
    }
    return false;
}
template <typename T, int N>
bool SlideCommand(const std::string& label, T (&value)[N], T min = T(), T max = T(), const char* format = "%.3f") {
    T newValue[N]; // 現在の値をコピー
    std::copy(value, value + N, newValue);
    if (SlideGui(label, newValue, min, max, format)) { // 値が変更された場合
        EditorGroup::getInstance()->pushCommand(std::make_unique<SetComamnd<T[N]>>(value, &newValue));
        return true;
    }
    return false;
}

template <typename T>
bool InputCommand(const std::string& label, T& value, const char* format = "%.3f") {
    T newValue = value; // 現在の値をコピー
    if (InputGui(label, newValue, format)) { // 値が変更された場合
        EditorGroup::getInstance()->pushCommand(std::make_unique<SetComamnd<T>>(value, &newValue));
        return true;
    }
    return false;
}
template <typename T, int N>
bool InputCommand(const std::string& label, T (&value)[N], const char* format = "%.3f") {
    T newValue[N]; // 現在の値をコピー
    std::copy(value, value + N, newValue);
    if (InputGui(label, newValue, format)) { // 値が変更された場合
        EditorGroup::getInstance()->pushCommand(std::make_unique<SetComamnd<T[N]>>(value, &newValue));
        return true;
    }
    return false;
}

#endif // _DEBUG
