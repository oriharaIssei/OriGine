#include "Model.h"

#include "texture/TextureManager.h"

namespace OriGine {

void Model::LoadTexture(int32_t part, const std::string& _texturePath) {
    materialData_[part].texturePath   = _texturePath;
    materialData_[part].textureNumber = TextureManager::LoadTexture(_texturePath);
}

void Skeleton::Update() {
    for (Joint& joint : this->joints) {
        joint.localMatrix = MakeMatrix4x4::Affine(joint.transform.scale, joint.transform.rotate, joint.transform.translate);

        if (joint.parent.has_value()) {
            joint.skeletonSpaceMatrix = joint.localMatrix * this->joints[*joint.parent].skeletonSpaceMatrix;
        } else {
            joint.skeletonSpaceMatrix = joint.localMatrix; // ルートジョイントはローカル行列がそのままスケルトン空間行列
        }
    }
}

void SkinCluster::UpdateMatrixPalette(const Skeleton& _skeleton) {
    for (size_t jointIndex = 0; jointIndex < _skeleton.joints.size(); ++jointIndex) {
        if (jointIndex >= this->inverseBindPoseMatrices.size()) {
            return;
        }
        auto& mat                            = this->skeletonMatrixPaletteBuffer_.openData_[jointIndex];
        mat.skeletonSpaceMat                 = this->inverseBindPoseMatrices[jointIndex] * _skeleton.joints[jointIndex].skeletonSpaceMatrix;
        mat.skeletonSpaceInverseTransposeMat = Matrix4x4::Transpose(Matrix4x4::Inverse(mat.skeletonSpaceMat));
    }

    this->skeletonMatrixPaletteBuffer_.ConvertToBuffer();
}

} // namespace OriGine
