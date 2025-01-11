#include "../manager/ParticleManager.h"
#include "Emitter.h"

//stl
//container
#include <array>

///engine
#include "Engine.h"
//module
#include "globalVariables/GlobalVariables.h"
#include "material/texture/TextureManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "model/ModelManager.h"
#include "myRandom/MyRandom.h"
#include "camera/CameraManager.h"
//assets
#include "EmitterShape.h"
#include "model/Model.h"
//object
#include "particle/Particle.h"

//math
#include <cmath>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

Emitter::Emitter(DxSrvArray* srvArray,const std::string& emitterName)
	:srvArray_(srvArray),
	emitterName_(emitterName.c_str()),
	originPos_{"Effects",emitterName,"originPos_"},
	modelFileName_{"Effects",emitterName,"modelFileName"},
	textureFileName_{"Effects",emitterName,"textureFileName"},
	shapeType_{"Effects",emitterName,"shapeType"},
	isLoop_{"Effects",emitterName,"isLoop"},
	activeTime_{"Effects",emitterName,"activeTime"},
	spawnCoolTime_{"Effects",emitterName,"spawnCoolTime"},
	particleLifeTime_{"Effects",emitterName,"particleLifeTime"},
	spawnParticleVal_{"Effects",emitterName,"spawnParticleVal"},
	particleMaxSize_{"Effects",emitterName,"particleMaxSize"},
	minDirectory_{"Effects",emitterName,"minDirectory"},
	maxDirectory_{"Effects",emitterName,"maxDirectory"},
	particleSpeed_{"Effects",emitterName,"particleSpeed"},
	particleScale_{"Effects",emitterName,"particleScale"},
	particleRotate_{"Effects",emitterName,"particleRotate"},
	particleUvScale_{"Effects",emitterName,"particleUvScale"},
	particleUvRotate_{"Effects",emitterName,"particleUvRotate"},
	particleUvTranslate_{"Effects",emitterName,"particleUvTranslate"},
	particleColor_{"Effects",emitterName,"particleColor"},
	particleIsBillBoard_{"Effects",emitterName,"particleIsBillBoard"}{
	isActive_ = false;
	leftActiveTime_ = 0.0f;
}

Emitter::~Emitter(){
	if(!particles_.empty()){
		particles_.clear();
	}
	structuredTransform_.Finalize();
}

static std::list<std::pair<std::string,std::string>> objectFiles = MyFileSystem::SearchFile("resource/Models","obj",false);
static std::list<std::pair<std::string,std::string>> textureFiles = MyFileSystem::SearchFile("resource/Models","png",false);

void Emitter::Init(){
	{ // Initialize DrawingData Size
		structuredTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice(),
										  srvArray_,
										  particleMaxSize_);
		particles_.reserve(particleMaxSize_);
	}

	{ // Initialize ShapeType
		switch(shapeType_.as<EmitterShapeType>()){
			case EmitterShapeType::SPHERE:
				emitterSpawnShape_ = std::make_unique<EmitterSphere>("Effects",emitterName_);
				break;
			case EmitterShapeType::AABB:
				emitterSpawnShape_ = std::make_unique<EmitterAABB>("Effects",emitterName_);
				break;
			default:
				break;
		}
	}

	{
		if(modelFileName_->c_str() != ""){
			particleModel_ = ModelManager::getInstance()->Create("resource/Models",modelFileName_);
		}
	}

	{ // Initialize Active State
		isActive_ = true;
		leftActiveTime_ = activeTime_;
	}
}

void Emitter::Update(float deltaTime){
	{ // Update Active
		if(!isActive_){
			return;
		}
		leftActiveTime_ -= deltaTime;
		// Loop するなら スキップ
		if(!isLoop_){
			// leftActiveTime が 0 以下で Particle が 全て消えたら
			if(leftActiveTime_ <= 0.0f && particles_.empty()){
				isActive_ = false;
				return;
			}
		}
	}

	{ // Particles Update
		for(auto& particle : particles_){
			particle->Update(deltaTime);
		}
		// isAliveでないもの は 消す
		std::erase_if(particles_,[](std::unique_ptr<Particle>& particle){return !particle->getIsAlive(); });
	}

	{ // Update Spawn 
		currentCoolTime_ -= deltaTime;
		if(currentCoolTime_ <= 0.0f){
			currentCoolTime_ = spawnCoolTime_;
			SpawnParticle();
		}
	}

	{ // push Drawing InstanceData
		structuredTransform_.openData_.clear();
		for(auto& particle : particles_){
			structuredTransform_.openData_.push_back(particle->getTransform());
		}
	}
}

#ifdef _DEBUG
void Emitter::Debug(){
	if(ImGui::Begin(emitterName_.c_str())){
		if(ImGui::Button("save")){
			GlobalVariables::getInstance()->SaveFile("Effects",emitterName_);
		}

		float deltaTime = Engine::getInstance()->getDeltaTime();
		ImGui::InputFloat("DeltaTime",&deltaTime,0.1f,1.0f,"%.3f",ImGuiInputTextFlags_ReadOnly);

		if(ImGui::Button("Active")){
			isActive_ = true;
			leftActiveTime_ = activeTime_;
		}
		ImGui::SameLine();
		if(ImGui::Button("Stop")){
			isActive_ = false;
		}
		ImGui::Text("Emitter Origin");
		ImGui::DragFloat3("##Emitter Origin",reinterpret_cast<float*>(originPos_.operator Vector3 * ()),0.1f);

		ImGui::Checkbox("isLoop",isLoop_);

		ImGui::Text("EmitterActiveTime");
		ImGui::DragFloat("##EmitterActiveTime",activeTime_,0.1f);
		ImGui::Text("SpawnParticleVal");
		ImGui::DragInt("##spawnParticleVal",spawnParticleVal_,1,0);

		ImGui::Checkbox("Particle Is BillBoard",particleIsBillBoard_);

		ImGui::Text("ParticleMaxSize");
		if(ImGui::Button("Calculate MaxSize")){
			CalculateMaxSize();
		}
		ImGui::SameLine();
		if(ImGui::InputInt("##particleMaxSize",particleMaxSize_,1,5,ImGuiInputTextFlags_EnterReturnsTrue)){
			structuredTransform_.Resize(Engine::getInstance()->getDxDevice()->getDevice(),particleMaxSize_);
		}

		ImGui::Text("SpawnCoolTime");
		ImGui::DragFloat("##SpawnCoolTime",spawnCoolTime_,0.1f,0);
		ImGui::Text("ParticleLifeTime");
		ImGui::DragFloat("##ParticleLifeTime",particleLifeTime_,0.1f,0);
		ImGui::Spacing();

		ImGui::Text("Particle minDirectory");
		ImGui::DragFloat3("##Particle minDirectory",reinterpret_cast<float*>(minDirectory_.operator Vector3 * ()),0.1f);
		ImGui::Text("Particle maxDirectory");
		ImGui::DragFloat3("##Particle maxDirectory",reinterpret_cast<float*>(maxDirectory_.operator Vector3 * ()),0.1f);

		minDirectory_.setValue({(std::min)(minDirectory_->x,maxDirectory_->x),
							   (std::min)(minDirectory_->y,maxDirectory_->y),
							   (std::min)(minDirectory_->z,maxDirectory_->z)});
		maxDirectory_.setValue({(std::max)(minDirectory_->x,maxDirectory_->x),
							   (std::max)(minDirectory_->y,maxDirectory_->y),
							   (std::max)(minDirectory_->z,maxDirectory_->z)});

		ImGui::Text("ParticleSpeed");
		ImGui::DragFloat("##ParticleSpeed",particleSpeed_,0.1f,0);
		ImGui::Spacing();
		if(ImGui::TreeNode("Particle InitialData")){
			ImGui::Text("Particle Color");
			ImGui::ColorEdit4("##Particle Color",reinterpret_cast<float*>(particleColor_.operator Vector4 * ()));
			ImGui::Text("Particle Scale");
			ImGui::DragFloat3("##Particle Scale",reinterpret_cast<float*>(particleScale_.operator Vector3 * ()),0.1f);
			ImGui::Text("Particle Rotate");
			ImGui::DragFloat3("##Particle Rotate",reinterpret_cast<float*>(particleRotate_.operator Vector3 * ()),0.1f);

			ImGui::Spacing();

			ImGui::Text("Particle UV Scale");
			ImGui::DragFloat3("##ParticleUvScale",reinterpret_cast<float*>(particleUvScale_.operator Vector3 * ()),0.1f);
			ImGui::Text("Particle UV Rotate");
			ImGui::DragFloat3("##ParticleUvRotate",reinterpret_cast<float*>(particleUvRotate_.operator Vector3 * ()),0.1f);
			ImGui::Text("Particle UV Translate");
			ImGui::DragFloat3("##ParticleUvTranslate",reinterpret_cast<float*>(particleUvTranslate_.operator Vector3 * ()),0.1f);

			ImGui::TreePop();
		}
		ImGui::Spacing();

		if(ImGui::Button("reload FileList")){
			objectFiles = MyFileSystem::SearchFile("resource/Models","obj",false);
			textureFiles = MyFileSystem::SearchFile("resource/Models","png",false);
		}

		if(ImGui::BeginCombo("ParticleModel",modelFileName_->c_str())){
			for(auto& fileName : objectFiles){
				bool isSelected = (fileName.second == modelFileName_); // 現在選択中かどうか
				if(ImGui::Selectable(fileName.second.c_str(),isSelected)){
					particleModel_ = ModelManager::getInstance()->Create(fileName.first,fileName.second);
					modelFileName_.setValue(fileName.second);
				}
			}
			ImGui::EndCombo();
		}
		if(ImGui::BeginCombo("ParticleTexture",textureFileName_->c_str())){
			for(auto& fileName : textureFiles){
				bool isSelected = (fileName.second == textureFileName_); // 現在選択中かどうか
				if(ImGui::Selectable(fileName.second.c_str(),isSelected)){
					textureFileName_.setValue(fileName.first + "/" + fileName.second);
					particleModel_->setTexture(0,TextureManager::LoadTexture(textureFileName_));
				}
			}
			ImGui::EndCombo();
		}

		if(ImGui::BeginCombo("EmitterShapeType",emitterShapeTypeWord_[shapeType_].c_str())){
			for(int32_t i = 0; i < shapeTypeCount; i++){
				bool isSelected = (shapeType_ == i); // 現在選択中かどうか

				if(ImGui::Selectable(emitterShapeTypeWord_[i].c_str(),isSelected)){
					switch(EmitterShapeType(i)){
						case EmitterShapeType::SPHERE:
							emitterSpawnShape_ = std::make_unique<EmitterSphere>("Effects",emitterName_);
							break;
						case EmitterShapeType::AABB:
							emitterSpawnShape_ = std::make_unique<EmitterAABB>("Effects",emitterName_);
							break;
						default:
							break;
					}
					shapeType_.setValue(i);
				}

				// 現在選択中の項目をハイライトする
				if(isSelected){
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if(ImGui::TreeNode("EmitterShape")){
			if(emitterSpawnShape_){
				emitterSpawnShape_->Debug();
			}
			ImGui::TreePop();
		}

	}
	ImGui::End();
}
#endif // _DEBUG

void Emitter::Draw(){
	if(!particleModel_ ||
        particleModel_->meshData_->currentState_ != LoadState::Loaded) {
		return;
	}

   const Matrix4x4& viewMat =   CameraManager::getInstance()->getTransform().viewMat;

	Matrix4x4 rotateMat = {};
	// パーティクルのスケール行列を事前計算
	Matrix4x4 scaleMat = MakeMatrix::Scale({1.0f,1.0f,1.0f});

	if(particleIsBillBoard_){ // Bill Board 
		// カメラの回転行列を取得し、平行移動成分をゼロにする
        Matrix4x4 cameraRotation = viewMat;
		cameraRotation[3][0] = 0.0f;
		cameraRotation[3][1] = 0.0f;
		cameraRotation[3][2] = 0.0f;
		cameraRotation[3][3] = 1.0f;

		// カメラの回転行列を反転してワールド空間への変換行列を作成
		rotateMat = cameraRotation.inverse();

		// 各パーティクルのワールド行列を計算
		for(size_t i = 0; i < particles_.size(); i++){
			scaleMat = MakeMatrix::Scale(structuredTransform_.openData_[i].scale);
			// 平行移動行列を計算
			Matrix4x4 translateMat = MakeMatrix::Translate(structuredTransform_.openData_[i].translate + originPos_);
			// ワールド行列を構築
			structuredTransform_.openData_[i].worldMat = scaleMat * rotateMat * translateMat;
		}
	} else{
		// 各パーティクルのワールド行列を計算
		for(size_t i = 0; i < particles_.size(); i++){
			scaleMat = MakeMatrix::Scale(structuredTransform_.openData_[i].scale);
			rotateMat = MakeMatrix::RotateXYZ(structuredTransform_.openData_[i].rotate);
			// 平行移動行列を計算
			Matrix4x4 translateMat = MakeMatrix::Translate(structuredTransform_.openData_[i].translate + originPos_);

			// ワールド行列を構築
			structuredTransform_.openData_[i].worldMat = scaleMat * rotateMat * translateMat;
		}
	}

	structuredTransform_.ConvertToBuffer();

	auto* commandList = ParticleManager::getInstance()->dxCommand_->getCommandList();
	uint32_t index = 0;
	for(auto& model : particleModel_->meshData_->mesh_){
		auto& material = particleModel_->materialData_[index];
		ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			3,
			TextureManager::getDescriptorGpuHandle(material.textureNumber)
		);

		commandList->IASetVertexBuffers(0,1,&model.meshBuff->vbView);
		commandList->IASetIndexBuffer(&model.meshBuff->ibView);

		structuredTransform_.SetForRootParameter(commandList,0);

		material.material->SetForRootParameter(commandList,2);
		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model.indexSize),static_cast<UINT>(structuredTransform_.openData_.size()),0,0,0);

		++index;
	}
}

void Emitter::CalculateMaxSize(){
	// 1秒あたりの生成回数
	float spawnRatePerSecond = spawnParticleVal_ / spawnCoolTime_;

	// 最大個数
	particleMaxSize_.setValue((std::max<int32_t>)(static_cast<int32_t>(std::ceil(spawnRatePerSecond * particleLifeTime_)),spawnParticleVal_));
}

void Emitter::SpawnParticle(){
	// スポーンして良い数 
	int32_t canSpawnParticleValue_ = (std::min<int32_t>)(spawnParticleVal_,static_cast<int32_t>(particleMaxSize_ - particles_.size()));

	for(int32_t i = 0; i < canSpawnParticleValue_; i++){
		//割りたてる Transform の 初期化
		structuredTransform_.openData_.push_back({});
		ParticleTransform& initialTransform = structuredTransform_.openData_.back();
		initialTransform.scale = particleScale_;
		initialTransform.rotate = particleRotate_;
		initialTransform.translate = emitterSpawnShape_->getSpawnPos();

		initialTransform.uvScale = particleUvScale_;
		initialTransform.uvRotate = particleUvRotate_;
		initialTransform.uvTranslate = particleUvTranslate_;

		initialTransform.color = particleColor_;

		Vector3 velocity = Vector3(
			MyRandom::Float(minDirectory_->x,maxDirectory_->x).get(),
			MyRandom::Float(minDirectory_->y,maxDirectory_->y).get(),
			MyRandom::Float(minDirectory_->z,maxDirectory_->z).get()
		);
		velocity *= MyRandom::Float(particleSpeed_,particleSpeed_).get();

		// Particle 初期化
		std::unique_ptr<Particle>& spawnedParticle = particles_.emplace_back<std::unique_ptr<Particle>>(std::make_unique<Particle>());
		spawnedParticle->Init(initialTransform,
							  velocity,
							  particleLifeTime_);
	}
}
