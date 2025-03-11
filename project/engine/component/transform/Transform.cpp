#include "Transform.h"

/// engine
#include <Engine.h>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

Transform::Transform(GameEntity* _hostEntity) : IComponent(_hostEntity), worldMat(MakeMatrix::Identity()) {}

void Transform::Init() {
    worldMat = MakeMatrix::Identity();
}

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
    _writer.WriteVec(scale);
    _writer.WriteVec(rotate);
    _writer.WriteVec(translate);
}

void Transform::Load(BinaryReader& _reader) {
    _reader.ReadVec(scale);
    _reader.ReadVec(rotate);
    _reader.ReadVec(translate);
    Update();
}
