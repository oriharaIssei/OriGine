#include "PrimitiveMeshRendererBase.h"

void PrimitiveMeshRendererBase::LoadTexture(const std::string& _directory, const std::string& _filename) {
    textureDirectory_ = _directory;
    textureFileName_  = _filename;
    textureIndex_     = TextureManager::LoadTexture(_directory + "/" + _filename);
}
