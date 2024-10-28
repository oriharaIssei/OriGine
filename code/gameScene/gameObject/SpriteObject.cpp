#include "SpriteObject.h"

#include "sprite/SpriteCommon.h"
#include "imgui/imgui.h"

#include "System.h"

void SpriteObject::Init([[maybe_unused]] const std::string &directory,const std::string &objectName){
	name_ = objectName;
	materialManager_ = System::getInstance()->getMaterialManager();

	sprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance()
	);
	sprite_->Init(directory + '/' + objectName + ".png");

	color_ = {1.0f,1.0f,1.0f,1.0f};
	pos_ = {0,0};
	size_ = {100.0f,100.0f};
}

void SpriteObject::Update(){
	sprite_->Debug(name_);
	sprite_->Update();
}

void SpriteObject::Draw(const IConstantBuffer<CameraTransform>&viewProj){
	viewProj;
	sprite_->Draw();
}