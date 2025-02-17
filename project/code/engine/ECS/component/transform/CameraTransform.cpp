#include "transform/CameraTransform.h"

void CameraTransform::UpdateMatrix(){
	viewMat = MakeMatrix::Affine({1.0f,1.0f,1.0f},rotate,translate);
	viewMat = viewMat.inverse();

	projectionMat = MakeMatrix::PerspectiveFov(fovAngleY,aspectRatio,nearZ,farZ);
}