#pragma once

/// stl
#include <cassert> // 追加
#include <string>

/// math
#include "Vector.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

/// externals
// imgui
#include <imgui/imgui.h>

enum class MyGuiLabelLocation {
    Upper,
    Left,
    Right
};

namespace MyGui {

// 変換用関数
inline Vec2f ToVec2f(const ImVec2& imVec) {
    return Vec2f(imVec.x, imVec.y);
}
inline ImVec2 ToImVec2(const Vec2f& vec) {
    return ImVec2(vec[X], vec[Y]);
}
inline Vec4f ToVec4f(const ImVec4& imVec) {
    return Vec4f(imVec.x, imVec.y, imVec.z, imVec.w);
}
inline ImVec4 ToImVec4(const Vec4f& vec) {
    return ImVec4(vec[X], vec[Y], vec[Z], vec[W]);
}

///====================================================================================
// Input
///====================================================================================
template <typename T>
bool Input(const std::string& _label, T* _value, T _step = T(1), T _stepFast = T(100), MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    // 定義されていない型の場合はエラーを返す
    return false;
}
template <typename T>
bool Input(const std::string& _label, T* _value, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    // 定義されていない型の場合はエラーを返す
    return false;
}

#pragma region "Input"
template <>
bool Input<int>(const std::string& _label, int* _value, int _step, int _stepFast, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputInt(label.c_str(), _value, _step, _stepFast);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputInt(label.c_str(), _value, _step, _stepFast);
    } else {
        return ImGui::InputInt(_label.c_str(), _value, _step, _stepFast);
    }
}
template <>
bool Input<int>(const std::string& _label, int* _value, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputInt(label.c_str(), _value);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputInt(label.c_str(), _value);
    } else {
        return ImGui::InputInt(_label.c_str(), _value);
    }
}

template <>
bool Input<float>(const std::string& _label, float* _value, float _step, float _stepFast, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputFloat(label.c_str(), _value, _step, _stepFast);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputFloat(label.c_str(), _value, _step, _stepFast);
    } else {
        return ImGui::InputFloat(_label.c_str(), _value, _step, _stepFast);
    }
}
template <>
bool Input<float>(const std::string& _label, float* _value, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputFloat(label.c_str(), _value);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputFloat(label.c_str(), _value);
    } else {
        return ImGui::InputFloat(_label.c_str(), _value);
    }
}

template <>
bool Input<double>(const std::string& _label, double* _value, double _step, double _stepFast, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputDouble(label.c_str(), _value, _step, _stepFast);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputDouble(label.c_str(), _value, _step, _stepFast);
    } else {
        return ImGui::InputDouble(_label.c_str(), _value, _step, _stepFast);
    }
}
template <>
bool Input<double>(const std::string& _label, double* _value, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputDouble(label.c_str(), _value);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputDouble(label.c_str(), _value);
    } else {
        return ImGui::InputDouble(_label.c_str(), _value);
    }
}

bool Input(const std::string& _label, std::string* _value, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputText(_label.c_str(), &_value->operator[](0), sizeof(char) * 64);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputText(_label.c_str(), &_value->operator[](0), sizeof(char) * 64);
    } else {
        return ImGui::InputText(_label.c_str(), &_value->operator[](0), sizeof(char) * 64);
    }
}

bool Input(
    const std::string& _label,
    std::string* _value,
    size_t _size,
    ImGuiInputTextFlags _flags        = 0,
    ImGuiInputTextCallback _callBack  = ImGuiInputTextCallback(0),
    MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputText(_label.c_str(), &_value->operator[](0), _size, _flags, _callBack);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputText(_label.c_str(), &_value->operator[](0), _size, _flags, _callBack);
    } else {
        return ImGui::InputText(_label.c_str(), &_value->operator[](0), _size, _flags, _callBack);
    }
}

template <int dim, typename valueType>
bool Input(
    const std::string& _label,
    Vec<dim, valueType>* _value,
    MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0),
    ImGuiInputTextFlags _flags        = ImGuiInputTextFlags(0)) {
    assert(false && "MyGui::Input for Vector<dim, ValueType> is not implemented");
    return false;
}

// int
template <int dim>
bool Input(
    const std::string& _label,
    Vec<dim, int>* _value,
    MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0),
    ImGuiInputTextFlags _flags        = ImGuiInputTextFlags(0)) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_S32, _value->v, dim, NULL, NULL, "%d", _flags);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_S32, _value->v, dim, NULL, NULL, "%d", _flags);
    } else {
        return ImGui::InputScalarN(label, ImGuiDataType_S32, _value->v, dim, NULL, NULL, "%d", _flags);
    }
}

// unsigned int
template <int dim>
bool Input(
    const std::string& _label,
    Vec<dim, unsigned int>* _value,
    MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0),
    ImGuiInputTextFlags _flags        = ImGuiInputTextFlags(0)) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_U32, _value->v, dim, NULL, NULL, "%d", _flags);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_U32, _value->v, dim, NULL, NULL, "%d", _flags);
    } else {
        return ImGui::InputScalarN(label, ImGuiDataType_U32, _value->v, dim, NULL, NULL, "%d", _flags);
    }
}

// float
template <int dim>
bool Input(
    const std::string& _label,
    Vec<dim, float>* _value,
    MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0),
    ImGuiInputTextFlags _flags        = ImGuiInputTextFlags(0)) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_Float, _value->v, dim, NULL, NULL, "%d", _flags);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_Float, _value->v, dim, NULL, NULL, "%d", _flags);
    } else {
        return ImGui::InputScalarN(label, ImGuiDataType_Float, _value->v, dim, NULL, NULL, "%d", _flags);
    }
}

// double
template <int dim>
bool Input(
    const std::string& _label,
    Vec<dim, float>* _value,
    MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0),
    ImGuiInputTextFlags _flags        = ImGuiInputTextFlags(0)) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_Double, _value->v, dim, NULL, NULL, "%d", _flags);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::InputScalarN(label, ImGuiDataType_Double, _value->v, dim, NULL, NULL, "%d", _flags);
    } else {
        return ImGui::InputScalarN(label, ImGuiDataType_Double, _value->v, dim, NULL, NULL, "%d", _flags);
    }
}
#pragma endregion

///====================================================================================
// Slider
///====================================================================================
template <typename T>
bool Slider(const std::string& _label, T* _value, T _min, T _max, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    return false;
}
template <typename T>
bool Slider(const std::string& _label, T* _value, T _min, T _max, const char* _format, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    return false;
}

#pragma region "Slider"
template <>
bool Slider<int>(const std::string& _label, int* _value, int _min, int _max, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderInt(label.c_str(), _value, _min, _max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderInt(label.c_str(), _value, _min, _max);
    } else {
        return ImGui::SliderInt(_label.c_str(), _value, _min, _max);
    }
}
template <>
bool Slider<int>(const std::string& _label, int* _value, int _min, int _max, const char* _format, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderInt(label.c_str(), _value, _min, _max, _format);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderInt(label.c_str(), _value, _min, _max, _format);
    } else {
        return ImGui::SliderInt(_label.c_str(), _value, _min, _max, _format);
    }
}

template <>
bool Slider<float>(const std::string& _label, float* _value, float _min, float _max, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderFloat(label.c_str(), _value, _min, _max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderFloat(label.c_str(), _value, _min, _max);
    } else {
        return ImGui::SliderFloat(_label.c_str(), _value, _min, _max);
    }
}
template <>
bool Slider<float>(const std::string& _label, float* _value, float _min, float _max, const char* _format, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderFloat(label.c_str(), _value, _min, _max, _format);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderFloat(label.c_str(), _value, _min, _max, _format);
    } else {
        return ImGui::SliderFloat(_label.c_str(), _value, _min, _max, _format);
    }
}
template <>
bool Slider<double>(const std::string& _label, double* _value, double _min, double _max, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, _value, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, _value, &_min, &_max);
    } else {
        return ImGui::SliderScalar(_label.c_str(), ImGuiDataType_Double, _value, &_min, &_max);
    }
}
template <>
bool Slider<double>(const std::string& _label, double* _value, double _min, double _max, const char* _format, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, _value, &_min, &_max, _format);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, _value, &_min, &_max, _format);
    } else {
        return ImGui::SliderScalar(_label.c_str(), ImGuiDataType_Double, _value, &_min, &_max, _format);
    }
}

// Vec<int> 用
template <int dim>
bool Slider(const std::string& _label, Vec<dim, int>* _value, int _min, int _max, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation::Upper) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_S32, _value->v, dim, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_S32, _value->v, dim, &_min, &_max);
    } else {
        return ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_S32, _value->v, dim, &_min, &_max);
    }
}

// Vec<float> 用
template <int dim>
bool Slider(const std::string& _label, Vec<dim, float>* _value, float _min, float _max, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation::Upper) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Float, _value->v, dim, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Float, _value->v, dim, &_min, &_max);
    } else {
        return ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_Float, _value->v, dim, &_min, &_max);
    }
}

// Vec<double> 用
template <int dim>
bool Slider(const std::string& _label, Vec<dim, double>* _value, double _min, double _max, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation::Upper) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, _value->v, dim, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::SliderScalarN(label.c_str(), ImGuiDataType_Double, _value->v, dim, &_min, &_max);
    } else {
        return ImGui::SliderScalarN(_label.c_str(), ImGuiDataType_Double, _value->v, dim, &_min, &_max);
    }
}
#pragma endregion

///====================================================================================
// Drag
///====================================================================================
template <typename T>
bool Drag(const std::string& _label, T* _value, float _vSpeed = 1.0f, T _min = T(0), T _max = T(0), MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    return false;
}

#pragma region "Drag"
template <>
bool Drag<int>(const std::string& _label, int* _value, float _vSpeed, int _min, int _max, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::DragInt(label.c_str(), _value, _vSpeed, _min, _max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::DragInt(label.c_str(), _value, _vSpeed, _min, _max);
    } else {
        return ImGui::DragInt(_label.c_str(), _value, _vSpeed, _min, _max);
    }
}

template <>
bool Drag<float>(const std::string& _label, float* _value, float _vSpeed, float _min, float _max, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::DragFloat(label.c_str(), _value, _vSpeed, _min, _max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::DragFloat(label.c_str(), _value, _vSpeed, _min, _max);
    } else {
        return ImGui::DragFloat(_label.c_str(), _value, _vSpeed, _min, _max);
    }
}

template <>
bool Drag<double>(const std::string& _label, double* _value, float _vSpeed, double _min, double _max, MyGuiLabelLocation _labelLocation) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::DragScalar(label.c_str(), ImGuiDataType_Double, _value, _vSpeed, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::DragScalar(label.c_str(), ImGuiDataType_Double, _value, _vSpeed, &_min, &_max);
    } else {
        return ImGui::DragScalar(_label.c_str(), ImGuiDataType_Double, _value, _vSpeed, &_min, &_max);
    }
}

// Vec<int> 用
template <int dim>
bool Drag(const std::string& _label, Vec<dim, int>* _value, float _vSpeed, int _min, int _max, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation::Upper) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::DragScalarN(label.c_str(), ImGuiDataType_S32, _value->v, dim, &_vSpeed, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::DragScalarN(label.c_str(), ImGuiDataType_S32, _value->v, dim, &_vSpeed, &_min, &_max);
    } else {
        return ImGui::DragScalarN(_label.c_str(), ImGuiDataType_S32, _value->v, dim, &_vSpeed, &_min, &_max);
    }
}

// Vec<float> 用
template <int dim>
bool Drag(const std::string& _label, Vec<dim, float>* _value, float _vSpeed, float _min, float _max, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation::Upper) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Float, _value->v, dim, &_vSpeed, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Float, _value->v, dim, &_vSpeed, &_min, &_max);
    } else {
        return ImGui::DragScalarN(_label.c_str(), ImGuiDataType_Float, _value->v, dim, &_vSpeed, &_min, &_max);
    }
}

// Vec<double> 用
template <int dim>
bool Drag(const std::string& _label, Vec<dim, double>* _value, float _vSpeed, double _min, double _max, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation::Upper) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, _value->v, dim, &_vSpeed, &_min, &_max);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::DragScalarN(label.c_str(), ImGuiDataType_Double, _value->v, dim, &_vSpeed, &_min, &_max);
    } else {
        return ImGui::DragScalarN(_label.c_str(), ImGuiDataType_Double, _value->v, dim, &_vSpeed, &_min, &_max);
    }
}
#pragma endregion

///====================================================================================
// Checkbox
///====================================================================================
bool CheckBox(const std::string& _label, bool* _isCheck = nullptr, MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::Checkbox(label.c_str(), _isCheck);
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::Checkbox(label.c_str(), _isCheck);
    } else {
        return ImGui::Checkbox(_label.c_str(), _isCheck);
    }
}

///====================================================================================
// Button
///====================================================================================
bool Button(const std::string& _label, const Vec2f& _size = Vec2f(0.f, 0.f), MyGuiLabelLocation _labelLocation = MyGuiLabelLocation(0)) {
    if (_labelLocation == MyGuiLabelLocation::Upper) {
        ImGui::Text(_label.c_str());
        std::string label = "##" + _label;
        return ImGui::Button(label.c_str(), ToImVec2(_size));
    } else if (_labelLocation == MyGuiLabelLocation::Left) {
        ImGui::Text(_label.c_str());
        ImGui::SameLine();
        std::string label = "##" + _label;
        return ImGui::Button(label.c_str(), ToImVec2(_size));
    } else {
        return ImGui::Button(_label.c_str(), ToImVec2(_size));
    }
}

}
