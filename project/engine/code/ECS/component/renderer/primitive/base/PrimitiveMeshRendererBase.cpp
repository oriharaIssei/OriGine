#include "PrimitiveMeshRendererBase.h"

using namespace OriGine;

void PrimitiveMeshRendererBase::LoadTexture(const std::string& _directory, const std::string& _filename) {
    textureFilePath_ = _directory + "/" + _filename;
    textureIndex_    = TextureManager::LoadTexture(textureFilePath_);
}

void PrimitiveMeshRendererBase::LoadTexture(const std::string& _filePath) {
    textureFilePath_ = _filePath;
    textureIndex_    = TextureManager::LoadTexture(textureFilePath_);
}
