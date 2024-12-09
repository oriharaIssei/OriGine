#include "Camera.h"

#include "Engine.h"

Camera* Camera::getInstance(){
	static Camera instance{};
	return &instance;
}

void Camera::Init(){
	cTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
}

void Camera::Finalize(){
	cTransform_.Finalize();
}
