#include "GameScene.h"

#include "debugCamera/debugCamera.h"

#include "GameObject/IGameObject.h"
#include "GameObject/ModelObject.h"
#include "GameObject/SphereObject.h"
#include "GameObject/SpriteObject.h"

#include <string>

#include "directX12/dxResource/rtv/DxRtvArray.h"
#include "directX12/dxResource/rtv/DxRtvArrayManager.h"
#include "directX12/dxResource/srv/DxSrvArray.h"
#include "directX12/dxResource/srv/DxSrvArrayManager.h"

#include "myFileSystem/MyFileSystem.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "System.h"
#include "texture/TextureManager.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

constexpr char dockingIDName[] = "ObjectsWindow";

GameScene::~GameScene(){
}

void GameScene::Init(){
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Init();

	debugCamera_->setViewTranslate({0.0f,0.0f,-12.0f});
	cameraBuff_.Init();
	input_ = Input::getInstance();

	sceneRtvArray_ = DxRtvArrayManager::getInstance()->Create(1);
	sceneSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

	sceneView_ = std::make_unique<RenderTexture>(System::getInstance()->getDxCommand(),sceneRtvArray_.get(),sceneSrvArray_.get());
	sceneView_->Init({1280.0f,720.0f},DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,{0.0f,0.0f,0.0f,0.0f});

	materialManager_ = System::getInstance()->getMaterialManager();

	textureList_ = myFs::SearchFile("./resource","png");
	objectList_ = myFs::SearchFile("./resource","obj");

}

void GameScene::Update(){
#ifdef _DEBUG
	debugCamera_->Update();
	debugCamera_->DebugUpdate();
	cameraBuff_.viewMat = debugCamera_->getCameraBuffer().viewMat;
	cameraBuff_.projectionMat = debugCamera_->getCameraBuffer().projectionMat;
	cameraBuff_.ConvertToBuffer();
#endif // _DEBUG

#ifdef _DEBUG
	if(ImGui::Begin("MaterialManager")){
		materialManager_->DebugUpdate();
	}
	ImGui::End();

	System::getInstance()->getDirectionalLight()->DebugUpdate();
	System::getInstance()->getPointLight()->DebugUpdate();
	System::getInstance()->getSpotLight()->DebugUpdate();

	if(ImGui::Begin("FileLists")){
		if(ImGui::TreeNode("TextureFiles")){
			ImGui::TreePop();
			if(ImGui::BeginChild("TextureFiles",ImVec2(250,128),true,ImGuiWindowFlags_HorizontalScrollbar)){
				for(auto& pngFile : textureList_){
					ImGui::Bullet();
					if(!ImGui::Button(pngFile.second.c_str())){
						continue;
					}
					std::unique_ptr<SpriteObject> sprite = std::make_unique<SpriteObject>();
					sprite->Init(pngFile.first,pngFile.second);
					gameObjects_.emplace_back(std::move(sprite));
				}
			}
			ImGui::EndChild();
		}
		// 高さ20pxのスペース
		ImGui::Dummy(ImVec2(0.0f,20.0f));

		if(ImGui::TreeNode("ModelList")){
			ImGui::TreePop();
			if(ImGui::BeginChild("ObjectFiles",ImVec2(250,128),true,ImGuiWindowFlags_HorizontalScrollbar)){
				for(auto& objFile : objectList_){
					ImGui::Bullet();
					if(!ImGui::Button(objFile.second.c_str())){
						continue;
					}
					std::unique_ptr<ModelObject> model = std::make_unique<ModelObject>();
					model->Init(objFile.first,objFile.second);
					gameObjects_.emplace_back(std::move(model));
				}
			}
			ImGui::EndChild();
		}

		ImGui::Dummy(ImVec2(0.0f,10.0f));
		if(ImGui::Button("Create Sphere")){
			std::unique_ptr<SphereObject> sprite = std::make_unique<SphereObject>();
			sprite->Init("","sphere");
			gameObjects_.emplace_back(std::move(sprite));
		}

		int32_t index = 0;
		ImGuiID dockingID = ImGui::GetID(dockingIDName);
		for(auto objectItr = gameObjects_.begin(); objectItr != gameObjects_.end(); ){
			auto& object = *objectItr;
			std::string label = "# " + std::to_string(index) + object->getName();
			++index;

			ImGui::SetNextWindowDockID(dockingID);
			if(ImGui::Begin(label.c_str())){
				object->Update();

				ImGui::Dummy(ImVec2(0.0f,7.0f));
				if(ImGui::Button("Delete this")){
					objectItr = gameObjects_.erase(objectItr);
				} else{
					++objectItr;
				}
			} else{
				++objectItr;
			}
			ImGui::End();
		}
	}
	ImGui::End();
#endif // _DEBUG

	railEditor_->Update();
}

void GameScene::Draw(){
	sceneView_->PreDraw();

	for(auto& object : gameObjects_){
		object->Draw(cameraBuff_);
	}

	railEditor_->Draw();

	sceneView_->PostDraw();

	System::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	System::getInstance()->ScreenPostDraw();
}