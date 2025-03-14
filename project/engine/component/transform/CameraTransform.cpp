#include "CameraTransform.h"

#include <imgui/imgui.h>

void CameraTransform::Initialize(GameEntity* _hostEntity) {
    _hostEntity;
    UpdateMatrix();
}

bool CameraTransform::Edit() {
    bool isChange = false;
    isChange |= ImGui::DragFloat3("Rotate", rotate.v, 0.1f);
    isChange |= ImGui::DragFloat3("Translate", translate.v, 0.1f);

    ImGui::Spacing();

    isChange |= ImGui::DragFloat("FovAngleY", &fovAngleY, 0.1f);
    isChange |= ImGui::DragFloat("AspectRatio", &aspectRatio, 0.1f);
    isChange |= ImGui::DragFloat("NearZ", &nearZ, 0.1f);
    isChange |= ImGui::DragFloat("FarZ", &farZ, 0.1f);
    if (isChange) {
        UpdateMatrix();
    }
    return isChange;
}

void CameraTransform::Save(BinaryWriter& _writer) {
    _writer.Write<3, float>(rotate);
    _writer.Write<3, float>(translate);

    _writer.Write(fovAngleY);
    _writer.Write(aspectRatio);

    _writer.Write(nearZ);
    _writer.Write(farZ);
}

void CameraTransform::Load(BinaryReader& _reader) {
    _reader.Read<3, float>(rotate);
    _reader.Read<3, float>(translate);

    _reader.Read(fovAngleY);
    _reader.Read(aspectRatio);

    _reader.Read(nearZ);
    _reader.Read(farZ);

    UpdateMatrix();
}

void CameraTransform::Finalize() {
}

void CameraTransform::UpdateMatrix() {
    viewMat = MakeMatrix::Affine({1.0f, 1.0f, 1.0f}, rotate, translate);
    viewMat = viewMat.inverse();

    projectionMat = MakeMatrix::PerspectiveFov(fovAngleY, aspectRatio, nearZ, farZ);
}
