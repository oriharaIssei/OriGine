#include "SpriteObject.h"

#include "imgui/imgui.h"

#include "System.h"

void SpriteObject::Init([[maybe_unused]] const std::string &directory,const std::string &objectName){
	name_ = objectName;
	materialManager_ = System::getInstance()->getMaterialManager();

	sprite_.reset(Sprite::Create({0,0},{100.0f,100.0f},directory + '/' + objectName + ".png"));

	color_ = {1.0f,1.0f,1.0f,1.0f};
	pos_ = {0,0};
	size_ = {100.0f,100.0f};
}

void SpriteObject::Update(){
	ImGui::Text("Name: %s",name_.c_str());

	ImGui::ColorEdit4("Color",&color_.x);
	sprite_->setColor(color_);

	ImGui::DragFloat2("UVScale",&sprite_->uvScale.x,0.1f);
	ImGui::DragFloat2("UVRotate",&sprite_->uvRotate.x,0.1f);
	ImGui::DragFloat2("UVTranslate",&sprite_->uvTranslate.x,0.1f);

	ImGui::DragFloat2("Position",&pos_.x,0.1f);
	ImGui::DragFloat2("Size",&size_.x,0.1f);
	sprite_->setPos(pos_);
	sprite_->setSize(size_);
}

void SpriteObject::Draw(const ViewProjection &viewProj){
	viewProj;
	sprite_->Draw();
}