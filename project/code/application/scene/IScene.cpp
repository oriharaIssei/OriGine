#include "IScene.h"

#include "SceneManager.h"

#include "engine/directX12/DxRtvArrayManager.h"
#include "engine/directX12/DxSrvArrayManager.h"

#include "Engine.h"

IScene::IScene(const std::string& sceneName):name_(sceneName){}

IScene::~IScene(){}
