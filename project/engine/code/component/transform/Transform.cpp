#include "Transform.h"

/// engine
#include <Engine.h>

#ifdef _DEBUG
#include "camera/CameraManager.h"

#include "mygui/MyGui.h"
#include <imgui/ImGuizmo/ImGuizmo.h>
#endif // _DEBUG

Transform::Transform() {}

void Transform::Initialize([[maybe_unused]] GameEntity* _entity) {
    this->Update();
}

void Transform::Update() {
    rotate = Quaternion::Normalize(rotate);
    // 修正: 局所変換行列を作成
    worldMat = MakeMatrix::Affine(scale, rotate, translate);
    // 修正: 親の行列との乗算順序を変更 (親行列を左側に乗算)
    if (parent) {
        worldMat *= parent->worldMat;
    }
}

bool Transform::Edit() {
#ifdef _DEBUG
    bool isChange = false;
    // --------------------------- scale --------------------------- //
    isChange |= DragGuiVectorCommand<3, float>("Scale", this->scale, 0.01f, {}, {}, "%.3f", [this](Vector<3, float>* /*_s*/) { this->Update(); });
    // --------------------------- rotate --------------------------- //
    isChange |= DragGuiVectorCommand<4, float>("Rotate", this->rotate, 0.01f, {}, {}, "%.3f", [this](Vector<4, float>* _r) { *_r = Quaternion::Normalize(*_r);this->Update(); });
    this->rotate = Quaternion::Normalize(this->rotate);
    // --------------------------- translate --------------------------- //
    isChange |= DragGuiVectorCommand<3, float>("Translate", this->translate, 0.01f, {}, {}, "%.3f", [this](Vector<3, float>* /*_t*/) { this->Update(); });

    this->Update();

    return isChange;
#else
    return false;
#endif // _DEBUG
}
