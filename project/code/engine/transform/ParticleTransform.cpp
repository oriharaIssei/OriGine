#include "ParticleTransform.h"

void ParticleTransform::UpdateMatrix(){
	worldMat = MakeMatrix::Affine(scale,rotate,translate);

	if(parent){
		worldMat *= parent->worldMat;
	}

}
