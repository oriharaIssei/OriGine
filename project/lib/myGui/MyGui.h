#pragma once

/// stl
#include <format>
#include <string>
// container
#include <map>
#include <unordered_map>
#include <vector>

/// math
#include "Vector.h"

/// externals
#include "imgui/imgui.h"

///================================================================================
// ImGui を 使いやすく,Engineで UndoRedoを実装するためのラッパー
///================================================================================
namespace MyGui {
///-------------------------------------------------------------------
// Input
///-------------------------------------------------------------------
template <typename T>
void Input(const std::string& label, T* value, const std::string& format = "%lf", ImGuiInputTextFlags flags = 0) {
    ImGui::InputScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value, nullptr, nullptr, format.c_str(), flags);
}
template <>
void Input<int>(const std::string& label, int* value, const std::string& format, ImGuiInputTextFlags flags) {
    ImGui::InputScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_S32, value, nullptr, nullptr, format.c_str(), flags);
}
template <>
void Input<unsigned int>(const std::string& label, unsigned int* value, const std::string& format, ImGuiInputTextFlags flags) {
    ImGui::InputScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_U32, value, nullptr, nullptr, format.c_str(), flags);
}
template <>
void Input<float>(const std::string& label, float* value, const std::string& format, ImGuiInputTextFlags flags) {
    ImGui::InputScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Float, value, nullptr, nullptr, format.c_str(), flags);
}

template <>
void Input<double>(const std::string& label, double* value, const std::string& format, ImGuiInputTextFlags flags) {
    ImGui::InputScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value, nullptr, nullptr, format.c_str(), flags);
}

// --------------- Vector ------------------ //
template <int N, typename T>
void InputVector(const std::string& label, Vector<N, T>* value, const std::string& format = "", ImGuiInputTextFlags flags = 0) {
    ImGui::InputScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value->v, N, nullptr, nullptr, format.c_str(), flags);
}
template <int N>
void InputVector(const std::string& label, Vector<N, int>* value, const std::string& format, ImGuiInputTextFlags flags = 0) {
    ImGui::InputScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_S32, value->v, N, nullptr, nullptr, format.c_str(), flags);
}
template <int N>
void InputVector(const std::string& label, Vector<N, unsigned int>* value, const std::string& format, ImGuiInputTextFlags flags = 0) {
    ImGui::InputScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_U32, value->v, N, nullptr, nullptr, format.c_str(), flags);
}
template <int N>
void InputVector(const std::string& label, Vector<N, float>* value, const std::string& format, ImGuiInputTextFlags flags = 0) {
    ImGui::InputScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Float, value->v, N, nullptr, nullptr, format.c_str(), flags);
}
template <int N>
void InputVector(const std::string& label, Vector<N, double>* value, const std::string& format, ImGuiInputTextFlags flags = 0) {
    ImGui::InputScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value->v, N, nullptr, nullptr, format.c_str(), flags);
}

// string
void InputString(const std::string& label, std::string* value, const std::string& format = "%s", ImGuiInputTextFlags flags = 0) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), format.c_str(), value->c_str());
    ImGui::InputText(label.c_str(), buffer, sizeof(buffer), flags);
    *value = buffer;
}

///-------------------------------------------------------------------
// Slider
///-------------------------------------------------------------------
template <typename T>
void Slider(const std::string& label, T* value, const T& min, const T& max, const std::string& format = "%lf", ImGuiSliderFlags flags = 0) {
    ImGui::SliderScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value, &min, &max, format.c_str(), flags);
}
template <>
void Slider<int>(const std::string& label, int* value, const int& min, const int& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_S32, value, &min, &max, format.c_str(), flags);
}
template <>
void Slider<unsigned int>(const std::string& label, unsigned int* value, const unsigned int& min, const unsigned int& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_U32, value, &min, &max, format.c_str(), flags);
}
template <>
void Slider<float>(const std::string& label, float* value, const float& min, const float& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Float, value, &min, &max, format.c_str(), flags);
}
template <>
void Slider<double>(const std::string& label, double* value, const double& min, const double& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value, &min, &max, format.c_str(), flags);
}

// --------------- Vector ------------------ //

template <int N, typename T>
void SliderVector(const std::string& label, Vector<N, T>* value, const Vector<N, T>& min, const Vector<N, T>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value->v, N, &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void SliderVector(const std::string& label, Vector<N, int>* value, const Vector<N, int>& min, const Vector<N, int>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_S32, value->v, N, &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void SliderVector(const std::string& label, Vector<N, unsigned int>* value, const Vector<N, unsigned int>& min, const Vector<N, unsigned int>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_U32, value->v, N, &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void SliderVector(const std::string& label, Vector<N, float>* value, const Vector<N, float>& min, const Vector<N, float>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Float, value->v, N, &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void SliderVector(const std::string& label, Vector<N, double>* value, const Vector<N, double>& min, const Vector<N, double>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::SliderScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value->v, N, &min.v[0], &max.v[0], format.c_str(), flags);
}

///-------------------------------------------------------------------
// Drag
///-------------------------------------------------------------------
template <typename T>
void Drag(const std::string& label, T* value, const T& v_speed, const T& min, const T& max, const std::string& format = "%lf", ImGuiSliderFlags flags = 0) {
    ImGui::DragScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value, v_speed, &min, &max, format.c_str(), flags);
}
template <>
void Drag<int>(const std::string& label, int* value, const int& v_speed, const int& min, const int& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_S32, value, v_speed, &min, &max, format.c_str(), flags);
}
template <>
void Drag<unsigned int>(const std::string& label, unsigned int* value, const unsigned int& v_speed, const unsigned int& min, const unsigned int& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_U32, value, v_speed, &min, &max, format.c_str(), flags);
}
template <>
void Drag<float>(const std::string& label, float* value, const float& v_speed, const float& min, const float& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Float, value, v_speed, &min, &max, format.c_str(), flags);
}
template <>
void Drag<double>(const std::string& label, double* value, const double& v_speed, const double& min, const double& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalar(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value, v_speed, &min, &max, format.c_str(), flags);
}

// --------------- Vector ------------------ //

template <int N, typename T>
void DragVector(const std::string& label, Vector<N, T>* value, const Vector<N, T>& v_speed, const Vector<N, T>& min, const Vector<N, T>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value->v, N, &v_speed.v[0], &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void DragVector(const std::string& label, Vector<N, int>* value, const Vector<N, int>& v_speed, const Vector<N, int>& min, const Vector<N, int>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_S32, value->v, N, &v_speed.v[0], &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void DragVector(const std::string& label, Vector<N, unsigned int>* value, const Vector<N, unsigned int>& v_speed, const Vector<N, unsigned int>& min, const Vector<N, unsigned int>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_U32, value->v, N, &v_speed.v[0], &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void DragVector(const std::string& label, Vector<N, float>* value, const Vector<N, float>& v_speed, const Vector<N, float>& min, const Vector<N, float>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Float, value->v, N, &v_speed.v[0], &min.v[0], &max.v[0], format.c_str(), flags);
}
template <int N>
void DragVector(const std::string& label, Vector<N, double>* value, const Vector<N, double>& v_speed, const Vector<N, double>& min, const Vector<N, double>& max, const std::string& format, ImGuiSliderFlags flags) {
    ImGui::DragScalarN(label.c_str(), ImGuiDataType_::ImGuiDataType_Double, value->v, N, &v_speed.v[0], &min.v[0], &max.v[0], format.c_str(), flags);
}

///-------------------------------------------------------------------
// Combo
///-------------------------------------------------------------------

template <typename T>
void Combo(const std::string& label, T* current_item, const std::vector<std::string>& items, ImGuiComboFlags flags = 0) {
    std::vector<const char*> item_ptrs;
    for (const auto& item : items) {
        item_ptrs.push_back(item.c_str());
    }
    ImGui::Combo(label.c_str(), current_item, item_ptrs.data(), static_cast<int>(item_ptrs.size()), flags);
}

template <typename T>
void Combo(const std::string& label, T* current_item, const std::map<T, std::string>& items, ImGuiComboFlags flags = 0) {
    std::vector<const char*> item_ptrs;
    for (const auto& item : items) {
        item_ptrs.push_back(item.second.c_str());
    }
    ImGui::Combo(label.c_str(), current_item, item_ptrs.data(), static_cast<int>(item_ptrs.size()), flags);
}
template <typename T>
void Combo(const std::string& label, T* current_item, const std::map<std::string, T>& items, ImGuiComboFlags flags = 0) {
    std::vector<const char*> item_ptrs;
    for (const auto& item : items) {
        item_ptrs.push_back(item.first.c_str());
    }
    ImGui::Combo(label.c_str(), current_item, item_ptrs.data(), static_cast<int>(item_ptrs.size()), flags);
}

template <typename T>
void Combo(const std::string& label, T* current_item, const std::unordered_map<T, std::string>& items, ImGuiComboFlags flags = 0) {
    std::vector<const char*> item_ptrs;
    for (const auto& item : items) {
        item_ptrs.push_back(item.second.c_str());
    }
    ImGui::Combo(label.c_str(), current_item, item_ptrs.data(), static_cast<int>(item_ptrs.size()), flags);
}
template <typename T>
void Combo(const std::string& label, T* current_item, const std::unordered_map<std::string, T>& items, ImGuiComboFlags flags = 0) {
    std::vector<const char*> item_ptrs;
    for (const auto& item : items) {
        item_ptrs.push_back(item.first.c_str());
    }
    ImGui::Combo(label.c_str(), current_item, item_ptrs.data(), static_cast<int>(item_ptrs.size()), flags);
}

///-------------------------------------------------------------------
// CheckBox
///-------------------------------------------------------------------
bool CheckBox(const std::string& label, bool* value) {
    return ImGui::Checkbox(label.c_str(), value);
}

///-------------------------------------------------------------------
// Color
///-------------------------------------------------------------------
template <int N>
void Color(const std::string& label, Vector<N, float>* value, ImGuiColorEditFlags flags = 0) {
    ImGui::ColorEdit4(label.c_str(), value->v, flags);
}
template <>
void Color<3>(const std::string& label, Vector<3, float>* value, ImGuiColorEditFlags flags) {
    ImGui::ColorEdit3(label.c_str(), value->v, flags);
}
template <>
void Color<4>(const std::string& label, Vector<4, float>* value, ImGuiColorEditFlags flags) {
    ImGui::ColorEdit4(label.c_str(), value->v, flags);
}

///-------------------------------------------------------------------
// Button
///-------------------------------------------------------------------
bool Button(const std::string& label, const ImVec2& size = ImVec2(0, 0)) {
    return ImGui::Button(label.c_str(), size);
}
bool SmallButton(const std::string& label) {
    return ImGui::SmallButton(label.c_str());
}

///-------------------------------------------------------------------
// Text
///-------------------------------------------------------------------
void Text(const std::string& text) {
    ImGui::Text("%s", text.c_str());
}
void TextColored(const std::string& text, const ImVec4& color) {
    ImGui::TextColored(color, "%s", text.c_str());
}
void TextWrapped(const std::string& text) {
    ImGui::TextWrapped("%s", text.c_str());
}
void TextUnformatted(const std::string& text) {
    ImGui::TextUnformatted(text.c_str());
}
void LabelText(const std::string& label, const std::string& text) {
    ImGui::LabelText(label.c_str(), "%s", text.c_str());
}
void BulletText(const std::string& text) {
    ImGui::BulletText("%s", text.c_str());
}
};
