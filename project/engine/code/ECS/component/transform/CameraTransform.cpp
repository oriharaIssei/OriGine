#include "CameraTransform.h"

#ifdef _DEBUG
#include <myGui/myGui.h>
#endif

void CameraTransform::Initialize(Entity* _hostEntity) {
    _hostEntity;
    UpdateMatrix();
}

void CameraTransform::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    DragGuiVectorCommand("Rotate##" + _parentLabel, rotate, 0.01f);
    DragGuiVectorCommand("Translate##" + _parentLabel, translate, 0.01f);

    ImGui::Spacing();

    DragGuiCommand("FovAngleY##" + _parentLabel, fovAngleY, 0.1f);
    DragGuiCommand("AspectRatio##" + _parentLabel, aspectRatio, 0.1f);
    DragGuiCommand("NearZ##" + _parentLabel, nearZ, 0.1f);
    DragGuiCommand("FarZ##" + _parentLabel, farZ, 0.1f);

    UpdateMatrix();

#endif // _DEBUG
}

void CameraTransform::Finalize() {}

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
