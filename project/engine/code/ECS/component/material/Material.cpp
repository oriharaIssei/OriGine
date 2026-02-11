#include "Material.h"

/// engine
#include "Engine.h"
#include "asset/AssetSystem.h"
// asset
#include "asset/TextureAsset.h"
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/ResourceStateTracker.h"

/// editor
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

UVTransform::ConstantBuffer& UVTransform::ConstantBuffer::operator=(const UVTransform& _transform) {
    uvTransform = MakeMatrix4x4::Affine({_transform.scale_, 1}, {0.f, 0.f, _transform.rotate_}, {_transform.translate_, 0.f});
    return *this;
}

ColorAndUvTransform::ConstantBuffer& ColorAndUvTransform::ConstantBuffer::operator=(const ColorAndUvTransform& _data) {
    color       = _data.color_;
    uvTransform = MakeMatrix4x4::Affine({_data.uvTransform_.scale_, 1}, {0.f, 0.f, _data.uvTransform_.rotate_}, {_data.uvTransform_.translate_, 0.f});
    return *this;
}

void Material::UpdateUvMatrix() {
    uvMat_ = MakeMatrix4x4::Affine({uvTransform_.scale_, 1}, {0.f, 0.f, uvTransform_.rotate_}, {uvTransform_.translate_, 0.f});
}

void Material::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {
    UpdateUvMatrix();
}

void Material::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    constexpr float kCustomTextureSize = 32.f;

    if (customTexture_.has_value()) {
        ImGui::Image(reinterpret_cast<ImTextureID>(customTexture_->srv_.GetGpuHandle().ptr), {kCustomTextureSize, kCustomTextureSize});
        ImGui::Spacing();
    }

    ImGui::Text("Color");
    ColorEditGuiCommand("##color" + _parentLabel, color_);

    ImGui::Spacing();

    ImGui::Text("uvScale");
    DragGuiVectorCommand<2, float>("##uvScale" + _parentLabel, uvTransform_.scale_, 0.01f);
    ImGui::Text("uvRotate");
    DragGuiCommand<float>("##uvRotate" + _parentLabel, uvTransform_.rotate_, 0.01f);
    ImGui::Text("uvTranslate");
    DragGuiVectorCommand<2, float>("##uvTranslate" + _parentLabel, uvTransform_.translate_, 0.01f);

    ImGui::Spacing();

    ImGui::Text("isLightUse");
    CheckBoxCommand("##isLightUse" + _parentLabel, enableLighting_);

    ImGui::Text("shininess");
    std::string label = "##shininess" + _parentLabel;
    ImGui::DragFloat(label.c_str(), &shininess_, 0.01f);
    ImGui::Text("specularColor");
    label = "##specularColor" + _parentLabel;
    ColorEditGuiCommand(label.c_str(), specularColor_);

    ImGui::Text("EnvironmentCoefficient");
    DragGuiCommand("##EnvironmentCoefficient" + _parentLabel, environmentCoefficient_, 0.01f, 0.0f);
#endif // _DEBUG
}

void Material::Finalize() {
    uvTransform_ = {};
    uvMat_       = MakeMatrix4x4::Identity();

    color_ = {1.f, 1.f, 1.f, 1.f};

    enableLighting_         = false;
    shininess_              = 0.f;
    environmentCoefficient_ = 0.1f;
    specularColor_          = {1.f, 1.f, 1.f};

    if (customTexture_.has_value()) {
        customTexture_->resource_.Finalize();
        auto srvHeap = Engine::GetInstance()->GetSrvHeap();
        srvHeap->ReleaseDescriptor(customTexture_->srv_);
    }
}

void Material::CreateCustomTextureFromMetaData(DirectX::TexMetadata& _metaData) {
    _metaData.format    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    _metaData.mipLevels = 1;

    // Resource & SRV の作成
    customTexture_.emplace(Material::CustomTextureData());

    customTexture_->resource_.CreateTextureResource(
        Engine::GetInstance()->GetDxDevice()->device_,
        _metaData);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels     = 1;

    /// SRV の作成
    SRVEntry srvEntry{&customTexture_->resource_, srvDesc};
    customTexture_->srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&srvEntry);

    /// Set ResourceStateTracker
    ResourceStateTracker::RegisterResource(customTexture_->resource_.GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
}

void Material::CreateCustomTextureFromTextureFile(const std::string& _directory, const std::string& _filename) {
    // metadataの取得
    size_t textureId = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(_directory + _filename);
    CreateCustomTextureFromTextureFile(textureId);
}

void Material::CreateCustomTextureFromTextureFile(size_t _textureIndex) {
    DirectX::TexMetadata metaData = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(_textureIndex).metaData;
    CreateCustomTextureFromMetaData(metaData);
}

void Material::DeleteCustomTexture() {
    if (customTexture_.has_value()) {
        customTexture_->resource_.Finalize();

        auto srvHeap = Engine::GetInstance()->GetSrvHeap();
        srvHeap->ReleaseDescriptor(customTexture_->srv_);
        customTexture_.reset();
    }
}

void OriGine::to_json(nlohmann::json& _j, const Material& _comp) {
    to_json<2, float>(_j["uvScale"], _comp.uvTransform_.scale_);
    to_json(_j["uvRotate"], _comp.uvTransform_.rotate_);
    to_json<2, float>(_j["uvTranslate"], _comp.uvTransform_.translate_);

    to_json<4, float>(_j["color"], _comp.color_);

    _j["enableLighting"] = static_cast<bool>(_comp.enableLighting_);
    _j["shininess"]      = _comp.shininess_;
    to_json<3, float>(_j["specularColor"], _comp.specularColor_);
}

void OriGine::from_json(const nlohmann::json& _j, Material& _comp) {
    _j.at("uvScale").get_to(_comp.uvTransform_.scale_);
    _j.at("uvRotate").get_to(_comp.uvTransform_.rotate_);
    _j.at("uvTranslate").get_to(_comp.uvTransform_.translate_);
    _j.at("color").get_to(_comp.color_);
    _j.at("enableLighting").get_to(_comp.enableLighting_);
    _j.at("shininess").get_to(_comp.shininess_);
    _j.at("specularColor").get_to(_comp.specularColor_);
}
