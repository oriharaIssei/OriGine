#pragma once

#ifdef _DEBUG
/// stl
#include <memory>
#include <string>

/// engine
#include "module/editor/EditorController.h"
#include "module/editor/IEditor.h"

/// lib
#include "logger/Logger.h"

/// util
#include "util/nameof.h"

/// externals
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

/// math
#include "math/Vector.h"

template <typename T>
class GuiValuePool {
public:
    GuiValuePool()  = default;
    ~GuiValuePool() = default;

public:
private:
    std::unordered_map<std::string, T> value_;

public:
    void setValue(const std::string& name, const T& value) {
        // 値がすでに存在する場合は何もしない
        auto it = value_.find(name);
        if (it != value_.end()) {
            return;
        }
        value_[name] = value;
    }
    T popValue(const std::string& name) {
        T returnValue{};
        auto it = value_.find(name);
        if (it != value_.end()) {

            returnValue = it->second;

            value_.erase(it); // 値を削除

        } else {
            LOG_ERROR("Value not found in pool: {} \n Type : {}", name, nameof<T>());
        }

        return returnValue; // デフォルト値を返す
    }
};

template <typename T>
bool DragGui(const std::string& label, T& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {

    if constexpr (std::is_same_v<T, int>) {
        return ImGui::DragInt(label.c_str(), &value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ImGui::DragFloat(label.c_str(), &value, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ImGui::DragScalar(label.c_str(), ImGuiDataType_Double, &value, speed, &min, &max, format);
    } else {
        return false; // サポートされていない型の場合
    }
}

template <int N, typename T>
bool DragVectorGui(const std::string& label, Vector<N, T>& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f") {

    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ImGui::DragInt2(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 3)
            return ImGui::DragInt3(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 4)
            return ImGui::DragInt4(label.c_str(), value.v, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ImGui::DragFloat2(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 3)
            return ImGui::DragFloat3(label.c_str(), value.v, speed, min, max, format);
        if constexpr (N == 4)
            return ImGui::DragFloat4(label.c_str(), value.v, speed, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value.v, 2, speed, &min, &max, format);
        if constexpr (N == 3)
            return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value.v, 3, speed, &min, &max, format);
        if constexpr (N == 4)
            return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, value.v, 4, speed, &min, &max, format);
    } else {
        return false; // サポートされていない型の場合
    }
}

template <typename T>
bool SlideGui(const std::string& label, T& value, T min = T(), T max = T(), const char* format = "%.3f") {

    if constexpr (std::is_same_v<T, int>) {
        return ImGui::SliderInt(label.c_str(), &value, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        return ImGui::SliderFloat(label.c_str(), &value, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, &value, &min, &max, format);
    } else {
        return false; // サポートされていない型の場合
    }
}
template <typename T, int N>
bool SlideVectorGui(const std::string& label, Vector<N, T>& value, T min = T(), T max = T(), const char* format = "%.3f") {

    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ImGui::SliderInt2(label.c_str(), value.v, min, max, format);
        if constexpr (N == 3)
            return ImGui::SliderInt3(label.c_str(), value.v, min, max, format);
        if constexpr (N == 4)
            return ImGui::SliderInt4(label.c_str(), value.v, min, max, format);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ImGui::SliderFloat2(label.c_str(), value.v, min, max, format);
        if constexpr (N == 3)
            return ImGui::SliderFloat3(label.c_str(), value.v, min, max, format);
        if constexpr (N == 4)
            return ImGui::SliderFloat4(label.c_str(), value.v, min, max, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value.v, 2, &min, &max, format);
        if constexpr (N == 3)
            return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value.v, 3, &min, &max, format);
        if constexpr (N == 4)
            return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, value.v, 4, &min, &max, format);
    } else {
        return false; // サポートされていない型の場合
    }
}

template <typename T>
bool InputGui(const std::string& label, T& value, const char* format = "%.3f") {

    if constexpr (std::is_same_v<T, int>) {
        return ImGui::InputInt(label.c_str(), &value);
    } else if constexpr (std::is_same_v<T, float>) {
        return ImGui::InputFloat(label.c_str(), &value, 0.0f, 0.0f, format);
    } else if constexpr (std::is_same_v<T, double>) {
        return ImGui::InputScalar(label.c_str(), ImGuiDataType_Double, &value, nullptr, nullptr, format);
    } else {
        return false; // サポートされていない型の場合
    }
}
template <typename T, int N>
bool InputVectorGui(const std::string& label, Vector<N, T>& value, const char* format = "%.3f") {

    if constexpr (std::is_same_v<T, int>) {
        if constexpr (N == 2)
            return ImGui::InputInt2(label.c_str(), value.v);
        if constexpr (N == 3)
            return ImGui::InputInt3(label.c_str(), value.v);
        if constexpr (N == 4)
            return ImGui::InputInt4(label.c_str(), value.v);
    } else if constexpr (std::is_same_v<T, float>) {
        if constexpr (N == 2)
            return ImGui::InputFloat2(label.c_str(), value.v, format);
        if constexpr (N == 3)
            return ImGui::InputFloat3(label.c_str(), value.v, format);
        if constexpr (N == 4)
            return ImGui::InputFloat4(label.c_str(), value.v, format);
    } else if constexpr (std::is_same_v<T, double>) {
        if constexpr (N == 2)
            return ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value.v, 2, nullptr, nullptr, format);
        if constexpr (N == 3)
            return ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value.v, 3, nullptr, nullptr, format);
        if constexpr (N == 4)
            return ImGui::InputScalarN(label.c_str(), ImGuiDataType_Double, value.v, 4, nullptr, nullptr, format);
    } else {
        return false; // サポートされていない型の場合
    }
}

template <typename T>
bool DragGuiCommand(const std::string& label, T& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = false;
    result      = DragGui(label, value, speed, min, max, format);

    if (ImGui::IsItemActive()) {
        valuePool.setValue(label, value);
    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = value;
        value      = valuePool.popValue(label);
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<T>>(&value, newValue, _afterFunc));
        result = true;
    }

    return result;
}
template <int N, typename T>
bool DragGuiVectorCommand(const std::string& label, Vector<N, T>& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<Vector<N, T>> valuePool;

    bool result = false;
    result      = DragVectorGui(label, value, speed, min, max, format);

    if (ImGui::IsItemActive()) {
        valuePool.setValue(label, value);
    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
        Vector<N, T> newValue = value;
        value                 = valuePool.popValue(label);
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<Vector<N, T>>>(&value, newValue, _afterFunc));
        result = true;
    }

    return result;

    return false;
}

template <typename T>
bool SlideCommand(const std::string& label, T& value, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = false;
    result      = SlideGui(label, value, min, max, format);

    if (ImGui::IsItemActive()) {
        valuePool.setValue(label, value);
    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = value;
        value      = valuePool.popValue(label);
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<T>>(&value, newValue, _afterFunc));
        return true;
    }

    return result;
}

template <typename T, int N>
bool SlideVectorCommand(const std::string& label, Vector<N, T>& value, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<Vector<N, T>> valuePool;

    bool result = false;
    result      = SlideVectorGui(label, value, min, max, format);

    if (ImGui::IsItemActive()) {
        valuePool.setValue(label, value);
    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
        Vector<N, T> newValue = value;
        value                 = valuePool.popValue(label);
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<Vector<N, T>>>(&value, newValue, _afterFunc));
        return true;
    }

    return result;
}

template <typename T>
bool InputGuiCommand(const std::string& label, T& value, const char* format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = false;
    result      = InputGui(label, value, format);

    if (ImGui::IsItemActive()) {
        valuePool.setValue(label, value);
    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = value;
        value      = valuePool.popValue(label);
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<T>>(&value, newValue, _afterFunc));
        return true;
    }

    return result;
}
template <typename T, int N>
bool InputVectorGuiCommand(const std::string& label, Vector<N, T>& value, const char* format = "%.3f", std::function<void(Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<Vector<N, T>> valuePool;

    bool result = false;
    result      = InputVectorGui(label, value, format);

    if (ImGui::IsItemActive()) {
        valuePool.setValue(label, value);
    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
        Vector<N, T> newValue = value;
        value                 = valuePool.popValue(label);
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<Vector<N, T>>>(&value, newValue, _afterFunc));
        return true;
    }

    return result;
}

template <int N>
bool ColorEditGui(const std::string& label, Vector<N, float>& value) {
    static_assert(N == 3 || N == 4, "ColorEditGui only supports 3 or 4 components (RGB or RGBA).");

    if constexpr (N == 3) {
        return ImGui::ColorEdit3(label.c_str(), value.v);
    } else if constexpr (N == 4) {
        return ImGui::ColorEdit4(label.c_str(), value.v);
    } else {
        return false; // サポートされていない場合
    }
}

template <int N>
bool ColorEditGuiCommand(const std::string& label, Vector<N, float>& value, std::function<void(Vector<N, float>*)> _afterFunc = nullptr) {
    static_assert(N == 3 || N == 4, "ColorEditGuiCommand only supports 3 or 4 components (RGB or RGBA).");

    static GuiValuePool<Vector<N, float>> valuePool;

    bool result = false;
    result      = ColorEditGui(label, value);

    if (ImGui::IsItemActive()) {
        valuePool.setValue(label, value);
    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
        Vector<N, float> newValue = value;
        value                     = valuePool.popValue(label);
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<Vector<N, float>>>(&value, newValue, _afterFunc));
        return true;
    }

    return result;
}

bool CheckBoxCommand(const std::string& label, bool& value);

bool ButtonCommand(const std::string& label, bool& value);

#endif // _DEBUG
