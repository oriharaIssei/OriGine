#include "DebugCamera.h"

#include "imgui/imgui.h"

#include <cmath>
#include <numbers>

void DebugCamera::Init(){
	input_ = Input::getInstance();
	currentState_.reset(new Neutral(this));
}

void DebugCamera::DebugUpdate(){
	if(ImGui::Begin("DebugCamera")){
		ImGui::DragFloat3("Rotate",&cameraBuff_.rotate[X],0.1f);
		ImGui::DragFloat3("Translate",&cameraBuff_.translate[X],0.1f);
	}
	ImGui::End();
}

void DebugCamera::Update(){
	if(currentState_){
		currentState_->Update();
	}
	cameraBuff_.UpdateMatrix();
}

void DebugCamera::Neutral::Update(){
	if(!(host_->input_->isPressKey(DIK_LALT) || host_->input_->isPressKey(DIK_RALT))){
		return;
	}
	if(host_->input_->isTriggerMouseButton(0) || host_->input_->isWheel()){
		host_->currentState_.reset(new TranslationState(host_));
		return;
	} else if(host_->input_->isTriggerMouseButton(1)){
		host_->currentState_.reset(new RotationState(host_));
		return;
	}
}

void DebugCamera::TranslationState::Update(){
	uint32_t state = 0;
	bool a = host_->input_->isPreWheel();
	bool b = host_->input_->isPressMouseButton(0);
	uint32_t c = (host_->input_->isPressKey(DIK_LALT) | host_->input_->isPressKey(DIK_RALT));
	state = (a)+(b * 2);
	state *= c;
	Vec3f velo = {};
	switch((TranslationType)state){
		case NONE:
			host_->currentState_.reset(new Neutral(host_));
			return;
		case Z_WHEEL:
			velo = {0.0f,0.0f,(float)host_->input_->getPreWheel() * 0.007f};
			break;
		case XY_MOUSEMOVE:
			velo = {host_->input_->getMouseVelocity() * 0.01f,0.0f};
			break;
		case XYZ_ALL:
			velo = {host_->input_->getMouseVelocity() * 0.01f,(float)host_->input_->getPreWheel() * 0.007f};
			break;
		default:
			break;
	}
	velo[Y] *= -1.0f;
	host_->cameraBuff_.translate += TransformNormal(velo,MakeMatrix::RotateXYZ(host_->cameraBuff_.rotate));
}

void DebugCamera::RotationState::Update(){
	constexpr float maxRad = std::numbers::pi_v<float>*2.0f;
	if(!host_->input_->isPressMouseButton(1) ||
	   !(host_->input_->isPressKey(DIK_LALT) ||
	   host_->input_->isPressKey(DIK_RALT))){
		host_->currentState_.reset(new Neutral(host_));
		return;
	}
	host_->cameraBuff_.rotate += Vec3f(host_->input_->getMouseVelocity()[Y],host_->input_->getMouseVelocity()[X],0.0f) * 0.01f;
	host_->cameraBuff_.rotate = {
		std::fmod(host_->cameraBuff_.rotate[X],maxRad),
		std::fmod(host_->cameraBuff_.rotate[Y],maxRad),
		std::fmod(host_->cameraBuff_.rotate[Z],maxRad)
	};
}
