#include "MaterialEffectPipeLine.h"

/// engine
#define ENGINE_INCLUDE
#include "EngineInclude.h"
#include "texture/TextureManager.h"

MaterialEffectPipeLine::MaterialEffectPipeLine() {}

MaterialEffectPipeLine::~MaterialEffectPipeLine() {}

void MaterialEffectPipeLine::Initialize(GameEntity* /*_entity*/) {
    if (!baseTexturePath_.empty()) {
        LoadBaseTexture(baseTexturePath_);
    }
}

void MaterialEffectPipeLine::Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) {}

void MaterialEffectPipeLine::Finalize() {}

void MaterialEffectPipeLine::LoadBaseTexture(const std::string& _path) {
    baseTexturePath_ = _path;
    baseTextureId_   = TextureManager::LoadTexture(baseTexturePath_);

    CreateEffectedTextureResourceByBaseTexture();
}

void MaterialEffectPipeLine::CreateEffectedTextureResourceByBaseTexture() {
    // Resource & SRV の作成
    const DirectX::TexMetadata& metaData = TextureManager::getTexMetadata(baseTextureId_);
    effectedTextureResource_.CreateTextureResource(
        Engine::getInstance()->getDxDevice()->getDevice(),
        metaData);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                  = metaData.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels     = UINT(metaData.mipLevels);

    /// SRV の作成
    effectedTextureSrv_ = Engine::getInstance()->getSrvHeap()->CreateDescriptor(srvDesc, &effectedTextureResource_);

    // テクスチャサイズの取得
    textureSize_[X] = static_cast<float>(metaData.width);
    textureSize_[Y] = static_cast<float>(metaData.height);
}

void to_json(nlohmann::json& j, const MaterialEffectPipeLine& c) {
}

void from_json(const nlohmann::json& j, MaterialEffectPipeLine& c) {
}
