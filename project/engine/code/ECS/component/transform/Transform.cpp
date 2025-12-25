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

void Transform::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {
    this->UpdateMatrix();
}

void Transform::UpdateMatrix() {
    rotate   = Quaternion::Normalize(rotate);
    worldMat = MakeMatrix4x4::Affine(scale, rotate, translate);
    if (parent) {
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

void Transform::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
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
