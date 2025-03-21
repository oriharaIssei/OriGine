#include "ParticleTransform.h"

void ParticleTransform::UpdateMatrix(){
	worldMat = MakeMatrix::Affine(scale,rotate,translate);
	uvMat    = MakeMatrix::Affine(uvScale,uvRotate,uvTranslate);

	if(parentWorldMat){
        worldMat *= *parentWorldMat;
	}
}
