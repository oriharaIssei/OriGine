#include "ParticleTransform.h"

void ParticleTransform::UpdateMatrix(){
	worldMat = MakeMatrix::Affine(scale,rotate,translate);
	uvMat    = MakeMatrix::Affine(uvScale,uvRotate,uvTranslate);

	if(parent){
		worldMat *= parent->worldMat;
	}
}
