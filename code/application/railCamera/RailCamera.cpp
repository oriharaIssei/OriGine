#include "RailCamera.h"



#include "Spline.h"
#include "System.h"

#include "globalVariables/GlobalVariables.h"
#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void RailCamera::Init(int32_t dimension){
	cameraBuff_.UpdateMatrix();
	object_.reset(Object3d::Create("resource","axis.obj"));
	object_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());

	dimension_ = dimension;

	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue("Game","RailCamera","offset",offset_);
	variables->addValue("Game","RailCamera","acceleration",acceleration_);
	variables->addValue("Game","RailCamera","minVelocity_",minVelocity_);
	variables->addValue("Game","RailCamera","maxVelocity_",maxVelocity_);
}

const float kGravity = -9.8f;
void RailCamera::Update(){
#ifdef _DEBUG
	ImGui::Begin("RailCamera");
	ImGui::InputFloat("currentDistance",&currentDistance_,0.0f,0.0f,"%.2f",ImGuiInputTextFlags_ReadOnly);
	ImGui::End();
#endif // _DEBUG

	float deltaTime = System::getInstance()->getDeltaTime();

	float physicalVelocity = velocity_ * deltaTime;

	currentDistance_ += physicalVelocity;

	// スプラインのループ処理（必要に応じて）
	if(currentDistance_ > spline_->GetTotalLength()){
		currentDistance_ -= spline_->GetTotalLength();
	}
	if(currentDistance_ < 0.0f){
		currentDistance_ += spline_->GetTotalLength();
	}

	// 現在の距離に対応するtを取得
	float t = spline_->GetTFromDistance(currentDistance_);

	// 現在の位置
	Vector3 eye = spline_->GetPosition(t);

	// 次の位置を取得して方向を計算
	float nextDistance = currentDistance_ + physicalVelocity;
	if(nextDistance > spline_->GetTotalLength()){
		nextDistance -= spline_->GetTotalLength();
	}
	float nextT = spline_->GetTFromDistance(nextDistance);
	Vector3 target = spline_->GetPosition(nextT);


	Vector3 direction = (target - eye).Normalize();

	// カメラの向きと位置を更新
	Transform& transform = object_->transform_.openData_;
	transform.rotate.y = std::atan2(direction.x,direction.z);
	Vector2 veloXZ = {direction.x,direction.z};
	transform.rotate.x = std::atan2(-direction.y,veloXZ.Length());

	transform.translate = eye + offset_;

	transform.UpdateMatrix();
	object_->transform_.ConvertToBuffer();
	cameraBuff_.viewMat = transform.worldMat.Inverse();

	// 速度 の 計算
	velocity_ += direction.y * acceleration_ * deltaTime;
	velocity_  = std::clamp(velocity_,minVelocity_,maxVelocity_);
}

void RailCamera::Draw(const IConstantBuffer<CameraTransform>& cameraBuff){
	object_->Draw(cameraBuff);
}

void RailCamera::SetSpline(Spline* _spline){ spline_ = _spline; }