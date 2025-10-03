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

void MaterialEffectPipeLine::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) {}

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
    j["baseTexturePath"] = c.baseTexturePath_;
    j["isActive"]        = c.isActive_;

    nlohmann::json effectList = nlohmann::json::array();
    for (const auto& effectData : c.effectEntityIdList_) {
        nlohmann::json effectJson;
        effectJson["effectType"] = static_cast<int>(effectData.effectType);
        effectJson["entityID"]   = effectData.entityID;
        effectList.push_back(effectJson);
    }
    j["effectEntityIdList"] = effectList;
}

void from_json(const nlohmann::json& j, MaterialEffectPipeLine& c) {
    if (j.contains("baseTexturePath")) {
        j.at("baseTexturePath").get_to(c.baseTexturePath_);
    }
    if (j.contains("isActive")) {
        j.at("isActive").get_to(c.isActive_);
    }
    c.effectEntityIdList_.clear();
    if (j.contains("effectEntityIdList")) {
        for (const auto& effectJson : j.at("effectEntityIdList")) {
            MaterialEffectPipeLine::EffectEntityData effectData;
            if (effectJson.contains("effectType")) {
                int effectTypeInt = 0;
                effectJson.at("effectType").get_to(effectTypeInt);
                effectData.effectType = static_cast<MaterialEffectType>(effectTypeInt);
            }
            if (effectJson.contains("entityID")) {
                effectJson.at("entityID").get_to(effectData.entityID);
            }
            c.effectEntityIdList_.push_back(effectData);
        }
    }
    if (!c.baseTexturePath_.empty()) {
        c.LoadBaseTexture(c.baseTexturePath_);
    }
}
