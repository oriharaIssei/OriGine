#include "MaterialEffectPipeLine.h"

/// engine
#include "texture/TextureManager.h"

MaterialEffectPipeLine::MaterialEffectPipeLine() {}

MaterialEffectPipeLine::~MaterialEffectPipeLine() {}

void MaterialEffectPipeLine::Initialize(GameEntity* /*_entity*/) {
    if (!baseTexturePath_.empty()) {
        baseTextureId_ = TextureManager::LoadTexture(baseTexturePath_);
    }
}

void MaterialEffectPipeLine::Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) {
}

void MaterialEffectPipeLine::Finalize() {}

void MaterialEffectPipeLine::LoadBaseTexture(const std::string& _path) {
    baseTexturePath_ = _path;
    baseTextureId_   = TextureManager::LoadTexture(baseTexturePath_);
}
