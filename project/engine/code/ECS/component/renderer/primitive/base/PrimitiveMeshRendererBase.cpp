#include "PrimitiveMeshRendererBase.h"

/// engine
// asset
#include "asset/TextureAsset.h"

using namespace OriGine;

void PrimitiveMeshRendererBase::LoadTexture(const std::string& _directory, const std::string& _filename) {
    textureFilePath_ = _directory + "/" + _filename;
    textureIndex_    = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(textureFilePath_);
}

void PrimitiveMeshRendererBase::LoadTexture(const std::string& _filePath) {
    textureFilePath_ = _filePath;
    textureIndex_    = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(textureFilePath_);
}
