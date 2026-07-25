#include "Transform.h"

/// engine
#include <Engine.h>

#ifdef _DEBUG
// camera
#include "camera/CameraManager.h"

/// gui
#include "mygui/MyGui.h"
#include <imgui/ImGuizmo/ImGuizmo.h>
#endif // _DEBUG

using namespace OriGine;

Transform::Transform() {}

void Transform::Initialize(Scene* /*_scene*/, const EntityHandle& /*_entity*/) {
    this->UpdateMatrix();
}

/// <summary>
/// scale/rotate/translate から worldMat を再構築する。
/// </summary>
/// <remarks>
/// 親がいる場合は親のworldMatを掛けて階層を反映するため、
/// この関数を呼ぶ前に親側のUpdateMatrix()が済んでいる必要がある。
/// 親より先に子を更新すると、子だけ1フレーム前の親の姿勢に追従することになる。
/// </remarks>
void Transform::UpdateMatrix() {
    // 回転の合成やGUIでの直接編集を繰り返すとクォータニオンの長さが1からずれ、
    // 行列に変換した際に意図しない拡大縮小が混ざる。毎回正規化して単位長を保つ
    rotate   = Quaternion::Normalize(rotate);
    worldMat = MakeMatrix4x4::Affine(scale, rotate, translate);
    if (parent) {
        // 行ベクトル(v * M)規約なので、ローカル変換を先に適用してから親の変換を掛ける。
        // 順序を逆にすると親の回転が子のローカル変換より先に効いてしまい、階層がねじれる
        worldMat *= parent->worldMat;
    }
}

Quaternion Transform::CalculateWorldRotate() const {
    if (parent) {
        return Quaternion::Normalize(parent->CalculateWorldRotate() * rotate);
    } else {
        return Quaternion::Normalize(rotate);
    }
}

void Transform::Edit(Scene* /*_scene*/, const EntityHandle& /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    // --------------------------- scale --------------------------- //
    DragGuiVectorCommand<3, float>("Scale##" + _parentLabel, this->scale, 0.01f, {}, {}, "%.3f", [this](Vector<3, float>* /*_s*/) { this->UpdateMatrix(); });
    // --------------------------- rotate --------------------------- //
    DragGuiVectorCommand<4, float>("Rotate##" + _parentLabel, this->rotate, 0.01f, {}, {}, "%.3f", [this](Vector<4, float>* _r) { *_r = Quaternion::Normalize(*_r);this->UpdateMatrix(); });
    this->rotate = Quaternion::Normalize(this->rotate);
    // --------------------------- translate --------------------------- //
    DragGuiVectorCommand<3, float>("Translate##" + _parentLabel, this->translate, 0.01f, {}, {}, "%.3f", [this](Vector<3, float>* /*_t*/) { this->UpdateMatrix(); });

    this->UpdateMatrix();

#endif // _DEBUG
}

Vec3f OriGine::Transform::FrontVector() const {
    return rotate.RotateVector(axisZ);
}

Vec3f OriGine::Transform::RightVector() const {
    return rotate.RotateVector(axisX);
}

Vec3f OriGine::Transform::UpVector() const {
    return rotate.RotateVector(axisY);
}
