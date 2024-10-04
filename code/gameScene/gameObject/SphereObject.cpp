#include "SphereObject.h"

#include "primitiveDrawer/PrimitiveDrawer.h"
#include "System.h"

#include "imgui.h"

#include "Vector3.h"

constexpr uint32_t kVertexSize = 16 * 16 * 4;
constexpr uint32_t kIndexSize = 16 * 16 * 6;

void SphereObject::Init([[maybe_unused]] const std::string &directoryPath,const std::string &objectName){
	IGameObject::Init(directoryPath,objectName);

	mesh_ = std::make_unique<PrimitiveObject3dMesh>();
	mesh_->Create(kVertexSize,kIndexSize);

	material_ = materialManager_->Create("white");

	transform_.Init();
}

void SphereObject::Update(){
	ImGui::Text("Name : %s",name_.c_str());
	IGameObject::Update();
}

void SphereObject::Draw(const ViewProjection &viewProj){
	constexpr int kSubDivision = 16;
	const float kLonEvery = static_cast<float>(M_PI) * 2.0f / static_cast<float>(kSubDivision);
	const float kLatEvery = static_cast<float>(M_PI) / static_cast<float>(kSubDivision);

	for(int latIndex = 0; latIndex < kSubDivision; ++latIndex){
		float lat = -static_cast<float>(M_PI) / 2.0f + (kLatEvery * latIndex);
		for(int lonIndex = 0; lonIndex < kSubDivision; ++lonIndex){
			float lon = lonIndex * kLonEvery;

			uint32_t startIndex = (latIndex * kSubDivision + lonIndex) * 6;
			uint32_t startVertex = (latIndex * kSubDivision + lonIndex) * 4;

			mesh_->indexData[startIndex]     = startVertex;
			mesh_->indexData[startIndex + 1] = startVertex + 1;
			mesh_->indexData[startIndex + 2] = startVertex + 2;

			mesh_->indexData[startIndex + 3] = startVertex + 1;
			mesh_->indexData[startIndex + 4] = startVertex + 3;
			mesh_->indexData[startIndex + 5] = startVertex + 2;

			// lb 0,0
			mesh_->vertData[startVertex].pos     = {Vector4(
				std::cosf(lat) * std::cosf(lon),
				std::sinf(lat),
				std::cosf(lat) * std::sinf(lon),
				1.0f)
			};
			mesh_->vertData[startVertex].normal= Vector3(
				mesh_->vertData[startVertex].pos.x,
				mesh_->vertData[startVertex].pos.y,
				mesh_->vertData[startVertex].pos.z
			).Normalize() * -1.0f;

			// lt 0,1
			mesh_->vertData[startVertex + 1].pos = {Vector4(
				std::cosf(lat + kLatEvery) * std::cosf(lon),
				std::sinf(lat + kLatEvery),
				std::cosf(lat + kLatEvery) * std::sinf(lon),
				1.0f)
			};
			mesh_->vertData[startVertex + 1].normal=Vector3(
				mesh_->vertData[startVertex + 1].pos.x,
				mesh_->vertData[startVertex + 1].pos.y,
				mesh_->vertData[startVertex + 1].pos.z
			).Normalize() * -1.0f;

			//rb 1,0
			mesh_->vertData[startVertex + 2].pos = {Vector4(
				std::cosf(lat) * std::cosf(lon + kLonEvery),
				std::sinf(lat),
				std::cosf(lat) * std::sinf(lon + kLonEvery),
				1.0f)
			};
			mesh_->vertData[startVertex + 2].normal=Vector3(
				mesh_->vertData[startVertex + 2].pos.x,
				mesh_->vertData[startVertex + 2].pos.y,
				mesh_->vertData[startVertex + 2].pos.z
			).Normalize() * -1.0f;

			// rt 1,1
			mesh_->vertData[startVertex + 3].pos = {Vector4(
				std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery),
				std::sinf(lat + kLatEvery),
				std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery),
				1.0f)
			};
			mesh_->vertData[startVertex + 3].normal=Vector3(
				mesh_->vertData[startVertex + 3].pos.x,
				mesh_->vertData[startVertex + 3].pos.y,
				mesh_->vertData[startVertex + 3].pos.z
			).Normalize() * -1.0f;
		}
	}

	ID3D12GraphicsCommandList *commandList = PrimitiveDrawer::dxCommand_->getCommandList();

	commandList->SetGraphicsRootSignature(PrimitiveDrawer::trianglePso_[(int)PrimitiveDrawer::currentBlendMode_]->rootSignature.Get());
	commandList->SetPipelineState(PrimitiveDrawer::trianglePso_[(int)PrimitiveDrawer::currentBlendMode_]->pipelineState.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->IASetVertexBuffers(0,1,&mesh_->vbView);
	commandList->IASetIndexBuffer(&mesh_->ibView);

	transform_.SetForRootParameter(commandList,0);
	viewProj.SetForRootParameter(commandList,1);
	material_->SetForRootParameter(commandList,2);
	System::getInstance()->getDirectionalLight()->SetForRootParameter(commandList,3);
	System::getInstance()->getPointLight()->SetForRootParameter(commandList,4);
	System::getInstance()->getSpotLight()->SetForRootParameter(commandList,5);
	commandList->DrawIndexedInstanced(
		kIndexSize,1,0,0,0
	);
};