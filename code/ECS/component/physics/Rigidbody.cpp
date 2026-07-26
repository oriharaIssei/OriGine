#include "Rigidbody.h"

/// externals
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"

#include "util/deltaTime/DeltaTimer.h"
#endif // _DEBUG

using namespace OriGine;

Rigidbody::Rigidbody() {}

void Rigidbody::Initialize(Scene* /*_scene*/, const EntityHandle& /*_entity*/) {}

/// <summary>
/// エディタ用編集UI。速度・加速度の積分そのものはこのコンポーネントではなく、
/// このデータを参照する物理更新システム側で行われる
/// </summary>
void Rigidbody::Edit(Scene* /*_scene*/, const EntityHandle& /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG
    CheckBoxCommand("isActive##" + _parentLabel, isActive_);

    DragGuiVectorCommand("acceleration##" + _parentLabel, acceleration_);
    DragGuiVectorCommand("velocity##" + _parentLabel, velocity_);
    DragGuiCommand("maxXZSpeed##" + _parentLabel, maxXZSpeed_);

    ImGui::Separator();

    CheckBoxCommand("useGravity##" + _parentLabel, useGravity_);
    DragGuiCommand("mass##" + _parentLabel, mass_);
    DragGuiCommand("maxFallSpeed##" + _parentLabel, maxFallSpeed_, 0.1f, 0.f, 10000.f, "%.3f");

    ImGui::Separator();

    DragGuiCommand("restitution##" + _parentLabel, restitution_, 0.01f);

    ImGui::Separator();

    CheckBoxCommand("isUsingLocalDeltaTime##" + _parentLabel, isUsingLocalDeltaTime_);
    std::string label = "localDeltaTimeName##" + _parentLabel;
    if (isUsingLocalDeltaTime_) {
        ImGui::InputText(label.c_str(), &localDeltaTimeName_);
    } else {
        localDeltaTimeName_ = "";
    }

#endif // _DEBUG
}

/// <summary>
/// 現在の速度・加速度などを読み取り専用で表示するデバッグ表示
/// </summary>
void Rigidbody::Debug() {
#ifdef _DEBUG
    ImGui::DragFloat3("acceleration", acceleration_.v, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ReadOnly);
    ImGui::Text("acceleration Speed : %.3f", acceleration_.length());
    ImGui::DragFloat3("velocity", velocity_.v, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ReadOnly);
    ImGui::Text("velocity Speed : %.3f", velocity_.length());
    ImGui::Separator();
    ImGui::Checkbox("Use Gravity", &useGravity_);
    ImGui::DragFloat("Mass", &mass_, 0.1f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_ReadOnly);
#endif // _DEBUG
}

void Rigidbody::Finalize() {}

/// <summary>
/// Rigidbody を JSON へ書き出す
/// </summary>
void OriGine::to_json(nlohmann::json& _j, const Rigidbody& _comp) {
    _j["isActive"]     = _comp.isActive_;
    _j["acceleration"] = _comp.acceleration_;
    _j["velocity"]     = _comp.velocity_;
    _j["maxXZSpeed"]   = _comp.maxXZSpeed_;
    _j["mass"]         = _comp.mass_;
    _j["useGravity"]   = _comp.useGravity_;
    _j["maxFallSpeed"] = _comp.maxFallSpeed_;
    _j["restitution"]  = _comp.restitution_;

    _j["isUsingLocalDeltaTime"] = _comp.isUsingLocalDeltaTime_;
    _j["localDeltaTimeName"]    = _comp.localDeltaTimeName_;
}
/// <summary>
/// JSON から Rigidbody を復元する。フィールド追加前の旧セーブデータとの互換性のため、
/// 新しく追加された項目は contains() で存在確認してから読み込む
/// </summary>
void OriGine::from_json(const nlohmann::json& _j, Rigidbody& _comp) {
    if (_j.contains("isActive")) {
        _j.at("isActive").get_to(_comp.isActive_);
    }
    _j.at("acceleration").get_to(_comp.acceleration_);
    _j.at("velocity").get_to(_comp.velocity_);
    if (_j.contains("maxXZSpeed")) {
        _j.at("maxXZSpeed").get_to(_comp.maxXZSpeed_);
    }

    _j.at("mass").get_to(_comp.mass_);
    _j.at("useGravity").get_to(_comp.useGravity_);
    if (_j.contains("maxFallSpeed")) {
        _j.at("maxFallSpeed").get_to(_comp.maxFallSpeed_);
    }

    // restitution が存在しない旧セーブデータでも読み込みに失敗しないよう、value() でデフォルト値付き取得する
    _comp.restitution_ = _j.value("restitution", 0.0f); // デフォルト値を 0.0f に設定

    if (_j.contains("isUsingLocalDeltaTime")) {
        _j.at("isUsingLocalDeltaTime").get_to(_comp.isUsingLocalDeltaTime_);
    }
    if (_j.contains("localDeltaTimeName")) {
        _j.at("localDeltaTimeName").get_to(_comp.localDeltaTimeName_);
    }
}
