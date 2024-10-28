#include "directX12/buffer/CameraTransform.h"

void CameraTransform::UpdateMatrix(){
	viewMat = MakeMatrix::Affine({1.0f,1.0f,1.0f},rotate,translate);
	viewMat = viewMat.Inverse();

	projectionMat = MakeMatrix::PerspectiveFov(fovAngleY,aspectRatio,nearZ,farZ);
}