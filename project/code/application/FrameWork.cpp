#include "FrameWork.h"

#include "Engine.h"
#include "globalVariables/GlobalVariables.h"

FrameWork::FrameWork(){}

FrameWork::~FrameWork(){}

void FrameWork::Init(){
	variables_ = GlobalVariables::getInstance();
	variables_->LoadAllFile();

	engine_ = Engine::getInstance();
	engine_->Init();
}

void FrameWork::Finalize(){
	engine_->Finalize();
}