#include "Model.h"

#include "texture/TextureManager.h"

void Model::loadTexture(int32_t part, const std::string& _texturePath) {
    materialData_[part].texturePath   = _texturePath;
    materialData_[part].textureNumber = TextureManager::LoadTexture(_texturePath);
}
