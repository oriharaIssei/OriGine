#include "Transform.h"

#include "directX12/DxFunctionHelper.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <Engine.h>

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
    isChange |= ImGui::DragFloat4("Rotate", this->rotate.v, 0.01f);
    // --------------------------- translate --------------------------- //
    isChange |= ImGui::DragFloat3("Translate", this->translate.v, 0.01f);

    if (isChange) {
        this->Update();
    }

    return isChange;
}

void Transform::Save(BinaryWriter& _writer) {
    _writer.Write(scale);
    _writer.Write(rotate);
    _writer.Write(translate);
}

void Transform::Load(BinaryReader& _reader) {
    _reader.Read<Vec3f>(scale);
    _reader.Read<Quaternion>(rotate);
    _reader.Read<Vec3f>(translate);
    Update();
}
