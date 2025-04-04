#include "Transform.h"

/// engine
#include <Engine.h>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

Transform::Transform() {}

void Transform::Initialize([[maybe_unused]] GameEntity* _entity) {}

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
    isChange |= ImGui::DragFloat3("Scale", this->scale.v, 0.01f);
    // --------------------------- rotate --------------------------- //
    if (ImGui::DragFloat4("Rotate", this->rotate.v, 0.01f)) {
        isChange     = true;
        this->rotate = Quaternion::Normalize(this->rotate);
    }
    // --------------------------- translate --------------------------- //
    isChange |= ImGui::DragFloat3("Translate", this->translate.v, 0.01f);

    if (isChange) {
        this->Update();
    }

    return isChange;
#else
    return false;
#endif // _DEBUG
}

void Transform::Save(BinaryWriter& _writer) {
    _writer.Write<3, float>("scale", scale);
    _writer.Write<4, float>("rotate", rotate);
    _writer.Write<3, float>("translate", translate);
}

void Transform::Load(BinaryReader& _reader) {
    _reader.Read<3, float>("scale", scale);
    _reader.Read<4, float>("rotate", rotate);
    _reader.Read<3, float>("translate", translate);
    Update();
}
