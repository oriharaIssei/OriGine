#include "CameraTransform.h"

/// editor
#ifdef _DEBUG
#include <myGui/myGui.h>
#endif

using namespace OriGine;

void CameraTransform::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {
    UpdateMatrix();
}

void CameraTransform::Edit(Scene* /*_scene*/, EntityHandle /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
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
    viewMat = MakeMatrix4x4::Affine({1.0f, 1.0f, 1.0f}, rotate, translate);
    viewMat = viewMat.inverse();

    projectionMat = MakeMatrix4x4::PerspectiveFov(fovAngleY, aspectRatio, nearZ, farZ);
}

void OriGine::to_json(nlohmann::json& _j, const CameraTransform& _comp) {
    _j = nlohmann::json{
        {"rotate", _comp.rotate},
        {"translate", _comp.translate},
        {"fovAngleY", _comp.fovAngleY},
        {"aspectRatio", _comp.aspectRatio},
        {"nearZ", _comp.nearZ},
        {"farZ", _comp.farZ},
    };
}

void OriGine::from_json(const nlohmann::json& _j, CameraTransform& _comp) {
    _j.at("rotate").get_to(_comp.rotate);
    _j.at("translate").get_to(_comp.translate);
    _j.at("fovAngleY").get_to(_comp.fovAngleY);
    _j.at("aspectRatio").get_to(_comp.aspectRatio);
    _j.at("nearZ").get_to(_comp.nearZ);
    _j.at("farZ").get_to(_comp.farZ);
    _comp.UpdateMatrix();
}
