#pragma once

#ifdef _DEBUG
/// stl
#include <memory>
#include <string>

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"

#include "logger/Logger.h"

/// util
#include "util/nameof.h"

/// externals
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

/// math
#include "math/Vector.h"

/// <summary>
/// 値を一時的に保存するプール
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
class GuiValuePool {
public:
    GuiValuePool()  = default;
    ~GuiValuePool() = default;

public:
private:
    std::unordered_map<std::string, T> value_;

public:
    /// <summary>
    /// 値をプールに設定
    /// </summary>
    /// <param name="name">名前</param>
    /// <param name="value">値</param>
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
    /// <param name="name">名前</param>
    /// <returns>存在すればtrue</returns>
    bool hasValue(const std::string& _name) const {
        return value_.find(_name) != value_.end();
    }
    /// <summary>
    /// 値を取得し、プールから削除する
    /// </summary>
    /// <param name="name">名前</param>
    /// <returns>取得した値</returns>
    T popValue(const std::string& _name) {
        T returnValue{};
        auto it = value_.find(_name);
        if (it != value_.end()) {

            returnValue = it->second;

            value_.erase(it); // 値を削除

        } else {
            LOG_ERROR("Value not found in pool: {} \n Type : {}", _name, nameof<T>());
        }

        return returnValue; // デフォルト値を返す
    }
};

namespace ImGui {
/// <summary>
/// std::string に対応した InputText
/// </summary>
bool InputText(const char* _label, std::string* _str, ImGuiInputTextFlags _flags = 0);
}

/// <summary>
/// ::ImGui::Drag を型Tに対応させたラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="speed"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <returns></returns>
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
        return false; // サポートされていない型の場合
    }
}

/// <summary>
/// ::ImGui::Drag を型TのN次元ベクトルに対応させたラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="N"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="speed"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <returns></returns>
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
        return false; // サポートされていない型の場合
    }
}

/// <summary>
/// ::ImGui::Slider を型Tに対応させたラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <returns></returns>
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
        return false; // サポートされていない型の場合
    }
}

/// <summary>
/// ::ImGui::Slider を型TのN次元ベクトルに対応させたラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="N"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <returns></returns>
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
        return false; // サポートされていない型の場合
    }
}

/// <summary>
/// ::ImGui::Input を型Tに対応させたラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="format"></param>
/// <returns></returns>
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
        return false; // サポートされていない型の場合
    }
}
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
        return false; // サポートされていない型の場合
    }
}
/// <summary>
/// ::ImGui::Drag を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="speed"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <param name="_afterFunc"></param>
/// <returns></returns>
template <typename T>
bool DragGuiCommand(const std::string& _label, T& _value, float _speed = 0.1f, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = false;
    result      = DragGui(_label, _value, _speed, _min, _max, _format);

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
/// <summary>
/// ::ImGui::Drag を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="N"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="speed"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <param name="_afterFunc"></param>
/// <returns></returns>
template <int N, typename T>
bool DragGuiVectorCommand(const std::string& _label, OriGine::Vector<N, T>& _value, float _speed = 0.1f, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = false;
    result      = DragVectorGui(_label, _value, _speed, _min, _max, _format);

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

/// <summary>
/// ::ImGui::Slider を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <param name="_afterFunc"></param>
/// <returns></returns>
template <typename T>
bool SlideGuiCommand(const std::string& _label, T& _value, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = false;
    result      = SlideGui(_label, _value, _min, _max, _format);

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

/// <summary>
/// ::ImGui::Slider を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="N"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="format"></param>
/// <param name="_afterFunc"></param>
/// <returns></returns>
template <typename T, int N>
bool SlideVectorCommand(const std::string& _label, OriGine::Vector<N, T>& _value, T _min = T(), T _max = T(), const char* _format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = false;
    result      = SlideVectorGui(_label, _value, _min, _max, _format);

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

/// <summary>
/// ::ImGui::Input を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="format"></param>
/// <param name="_afterFunc"></param>
/// <returns></returns>
template <typename T>
bool InputGuiCommand(const std::string& _label, T& _value, const char* _format = "%.3f", std::function<void(T*)> _afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = false;
    result      = InputGui(_label, _value, _format);

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
/// <summary>
/// ::ImGui::Input を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="N"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="format"></param>
/// <param name="_afterFunc"></param>
/// <returns></returns>
template <typename T, int N>
bool InputVectorGuiCommand(const std::string& _label, OriGine::Vector<N, T>& _value, const char* _format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> _afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = false;
    result      = InputVectorGui(_label, _value, _format);

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

/// <summary>
/// ::ImGui::ColorEdit を型Nの3または4次元ベクトルに対応させたラッパー
/// </summary>
/// <typeparam name="N"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <returns></returns>
template <int N>
bool ColorEditGui(const std::string& _label, OriGine::Vector<N, float>& _value, ImGuiColorEditFlags _colorEditFlags = 0) {
    static_assert(N == 3 || N == 4, "ColorEditGui only supports 3 or 4 components (RGB or RGBA).");

    if constexpr (N == 3) {
        return ::ImGui::ColorEdit3(_label.c_str(), _value.v, _colorEditFlags);
    } else if constexpr (N == 4) {
        return ::ImGui::ColorEdit4(_label.c_str(), _value.v, _colorEditFlags);
    } else {
        return false; // サポートされていない場合
    }
}

/// <summary>
/// ::ImGui::ColorEdit を型Nの3または4次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <typeparam name="N"></typeparam>
/// <param name="label"></param>
/// <param name="value"></param>
/// <param name="_afterFunc"></param>
/// <returns></returns>
template <int N>
bool ColorEditGuiCommand(const std::string& _label, OriGine::Vector<N, float>& _value, ImGuiColorEditFlags _colorEditFlags = 0, std::function<void(OriGine::Vector<N, float>*)> _afterFunc = nullptr) {
    static_assert(N == 3 || N == 4, "ColorEditGuiCommand only supports 3 or 4 components (RGB or RGBA).");

    static GuiValuePool<OriGine::Vector<N, float>> valuePool;

    bool result = false;
    result      = ColorEditGui<N>(_label, _value, _colorEditFlags);

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

/// <summary>
/// ::ImGui::CheckBox をboolに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <param name="label"></param>
/// <param name="value"></param>
/// <returns></returns>
bool CheckBoxCommand(const std::string& _label, bool& _value);

/// <summary>
/// ::ImGui::Button をboolに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <param name="label"></param>
/// <param name="value"></param>
/// <returns></returns>
bool ButtonCommand(const std::string& _label, bool& _value);

/// <summary>
/// マウス座標をシーンビューの座標に変換する
/// </summary>
/// <param name="mousePos"></param>
/// <param name="sceneViewPos"></param>
/// <param name="sceneViewSize"></param>
/// <param name="originalResolution"></param>
/// <returns></returns>
OriGine::Vec2f ConvertMouseToSceneView(const OriGine::Vec2f& _mousePos, const ImVec2& _sceneViewPos, const ImVec2& _sceneViewSize, const OriGine::Vec2f& _originalResolution);

/// <summary>
/// テクスチャ読み込みボタンを表示する
/// </summary>
/// <param name="_texIndex"></param>
/// <param name="_parentLabel"></param>
/// <returns></returns>
bool AskLoadTextureButton(int32_t _texIndex, const std::string& _parentLabel);

/// <summary>
/// dialogを開いてファイルを選択させる(directoryとfilenameを分ける)
/// </summary>
/// <param name="_baseDirectory"></param>
/// <param name="_outputDirectory"></param>
/// <param name="_outputFileName"></param>
/// <param name="_extension">対象のファイル拡張子</param>
/// <param name="_withoutExtension">出力に拡張子を含めるかどうか. true = 含めない,false = 含める</param>
/// <param name="_withoutBaseDirectory">出力に_baseDirectoryを含めるかどうか. true ＝ 含めない, false = 含める</param>
/// <returns>選択に成功したかどうか. true = 成功した, false = 失敗した</returns>
bool OpenFileDialog(const std::string& _baseDirectory, std::string& _outputDirectory, std::string& _outputFileName, const std::vector<std::string>& _extension, bool _withoutExtension, bool _withoutBaseDirectory);

/// <summary>
/// dialogを開いてファイルを選択させる(directoryとfilenameを合わせる)
/// </summary>
/// <param name="_baseDirectory"></param>
/// <param name="_outputPath">出力</param>
/// <param name="_extension">対象のファイル拡張子</param>
/// <param name="_withoutExtension">出力に拡張子を含めるかどうか. true = 含めない,false = 含める</param>
/// <param name="_withoutBaseDirectory">出力に_baseDirectoryを含めるかどうか. true ＝ 含めない, false = 含める</param>
/// <returns>選択に成功したかどうか. true = 成功した, false = 失敗した</returns>
bool OpenFileDialog(const std::string& _baseDirectory, std::string& _outputPath, const std::vector<std::string>& _extension, bool _withoutExtension = false, bool _withoutBaseDirectory = false);

#endif // _DEBUG
