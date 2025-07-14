#include "CameraTransform.h"

#include <imgui/imgui.h>

void CameraTransform::Initialize(GameEntity* _hostEntity) {
    _hostEntity;
    UpdateMatrix();
}

bool CameraTransform::Edit() {
    bool isChange = false;
    isChange |= ImGui::DragFloat4("Rotate", rotate.v, 0.1f);
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

void CameraTransform::Finalize() {
}

void CameraTransform::UpdateMatrix() {
    rotate  = rotate.normalize();
    viewMat = MakeMatrix::Affine({1.0f, 1.0f, 1.0f}, rotate, translate);
    viewMat = viewMat.inverse();

    projectionMat = MakeMatrix::PerspectiveFov(fovAngleY, aspectRatio, nearZ, farZ);
}

void to_json(nlohmann::json& j, const CameraTransform& r) {
    j = nlohmann::json{
        {"rotate", r.rotate},
        {"translate", r.translate},
        {"fovAngleY", r.fovAngleY},
        {"aspectRatio", r.aspectRatio},
        {"nearZ", r.nearZ},
        {"farZ", r.farZ},
    };
}

void from_json(const nlohmann::json& j, CameraTransform& r) {
    j.at("rotate").get_to(r.rotate);
    j.at("translate").get_to(r.translate);
    j.at("fovAngleY").get_to(r.fovAngleY);
    j.at("aspectRatio").get_to(r.aspectRatio);
    j.at("nearZ").get_to(r.nearZ);
    j.at("farZ").get_to(r.farZ);
    r.UpdateMatrix();
}
