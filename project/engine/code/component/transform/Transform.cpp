#include "Transform.h"

/// engine
#include <Engine.h>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

Transform::Transform(){}

void Transform::Initialize([[maybe_unused]]GameEntity* _entity) {}

void Transform::Update() {
    worldMat = MakeMatrix::Affine(scale, rotate, translate);

    if (parent) {
        worldMat = parent->worldMat * worldMat;
    }
}

bool Transform::Edit() {
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
}

void Transform::Save(BinaryWriter& _writer) {
    _writer.Write<3, float>(scale);
    _writer.Write<4, float>(rotate);
    _writer.Write<3, float>(translate);
}

void Transform::Load(BinaryReader& _reader) {
    _reader.Read<3, float>(scale);
    _reader.Read<4, float>(rotate);
    _reader.Read<3, float>(translate);
    Update();
}
