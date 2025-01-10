#include "primitiveDrawer/PrimitiveDrawer.h"

#include "Engine.h"
#include "camera/CameraManager.h"
#include "directX12/DxFunctionHelper.h"

#include <Vector4.h>

std::unique_ptr<DxCommand> PrimitiveDrawer::dxCommand_;

// std::array<PipelineStateObj*,kBlendNum> PrimitiveDrawer::trianglePso_;
// std::array<std::string,kBlendNum> PrimitiveDrawer::trianglePsoKeys_;

std::array<PipelineStateObj*, kBlendNum> PrimitiveDrawer::linePso_;
std::array<std::string, kBlendNum> PrimitiveDrawer::linePsoKeys_;

std::unique_ptr<PrimitiveObject3dMesh> PrimitiveDrawer::lineMesh_ = nullptr;
uint32_t PrimitiveDrawer::lineInstanceVal_                        = 0;

// std::unique_ptr<PrimitiveObject3dMesh> PrimitiveDrawer::triangleMesh_ = nullptr;
// uint32_t PrimitiveDrawer::triangleInstanceVal_ = 0;

// std::unique_ptr<PrimitiveObject3dMesh>PrimitiveDrawer::quadMesh_ = nullptr;
// uint32_t PrimitiveDrawer::quadInstanceVal_ = 0;

BlendMode PrimitiveDrawer::currentBlendMode_ = BlendMode::Alpha;

void PrimitiveDrawer::Init() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Init(Engine::getInstance()->getDxDevice()->getDevice(), "main", "main");

    /*trianglePsoKeys_ = {
		"Prim_Blend_None",
		"Prim_Blend_Normal",
		"Prim_Blend_Add",
		"Prim_Blend_Sub",
		"Prim_Blend_Multiply",
		"Prim_Blend_Screen"
	};*/

    linePsoKeys_ = {
        "Line_Blend_None",
        "Line_Blend_Normal",
        "Line_Blend_Add",
        "Line_Blend_Sub",
        "Line_Blend_Multiply",
        "Line_Blend_Screen"};

    currentBlendMode_ = BlendMode::Alpha;

    CreatePso();

    lineMesh_ = std::make_unique<PrimitiveObject3dMesh>();
    lineMesh_->Create(2 * 600, 0);
    lineInstanceVal_ = 0;

    // triangleMesh_ = std::make_unique<PrimitiveObject3dMesh>();
    // triangleMesh_->Create(3 * 400,0);
    // triangleInstanceVal_ = 0;
    //
    // quadMesh_ = std::make_unique<PrimitiveObject3dMesh>();
    // quadMesh_->Create(4 * 1280,6 * 1280);
    // quadInstanceVal_ = 0;
}

void PrimitiveDrawer::Finalize() {
    dxCommand_->Finalize();

    lineMesh_->Finalize();
    // triangleMesh_->Finalize();
    // quadMesh_->Finalize();
}

void PrimitiveDrawer::Line(const Vec3f& p0, const Vec3f& p1, const IConstantBuffer<Transform>& Transform, const IConstantBuffer<Material>* material) {
    ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();

    const uint32_t startIndex                  = lineInstanceVal_ * 2;
    lineMesh_->vertData[startIndex].pos        = {p0.x(), p0.y(), p0.z(), 1.0f};
    lineMesh_->vertData[startIndex].normal     = p0;
    lineMesh_->vertData[startIndex + 1].pos    = {p1.x(), p1.y(), p1.z(), 1.0f};
    lineMesh_->vertData[startIndex + 1].normal = p1;

    lineMesh_->indexData[startIndex]     = startIndex;
    lineMesh_->indexData[startIndex + 1] = startIndex + 1;

    Transform.SetForRootParameter(commandList, 0);
    material->SetForRootParameter(commandList, 2);

    commandList->DrawIndexedInstanced(
        2, 1, startIndex * 2, 0, 0);
    ++lineInstanceVal_;
}

//
//void PrimitiveDrawer::Triangle(const Vec3f& p0,
//							   const Vec3f& p1,
//							   const Vec3f& p2,
//							   const  IConstantBuffer<Transform>& Transform,
//							   const IConstantBuffer<CameraTransform>& viewProj,
//							   const IConstantBuffer<Material>* material){
//	ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();
//
//	const uint32_t startIndex = triangleInstanceVal_ * 3;
//	triangleMesh_->vertData[startIndex].pos = {p0.x,p0.y,p0.z,1.0f};
//	triangleMesh_->vertData[startIndex].normal = p0;
//	triangleMesh_->vertData[startIndex + 1].pos = {p1.x,p1.y,p1.z,1.0f};
//	triangleMesh_->vertData[startIndex + 1].normal = p1;
//	triangleMesh_->vertData[startIndex + 2].pos = {p2.x,p2.y,p2.z,1.0f};
//	triangleMesh_->vertData[startIndex + 2].normal = p2;
//
//	triangleMesh_->indexData[startIndex] = startIndex;
//	triangleMesh_->indexData[startIndex + 1] = startIndex + 1;
//	triangleMesh_->indexData[startIndex + 2] = startIndex + 2;
//
//	commandList->SetGraphicsRootSignature(trianglePso_[(int)currentBlendMode_]->rootSignature.Get());
//	commandList->SetPipelineState(trianglePso_[(int)currentBlendMode_]->pipelineState.Get());
//
//	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	commandList->IASetVertexBuffers(0,1,&triangleMesh_->vbView);
//	commandList->IASetIndexBuffer(&triangleMesh_->ibView);
//
//	Transform.SetForRootParameter(commandList,0);
//	viewProj.SetForRootParameter(commandList,1);
//	material->SetForRootParameter(commandList,2);
//	Engine::getInstance()->getLightManager()->SetForRootParameter(commandList);
//
//	commandList->DrawIndexedInstanced(
//		3,1,startIndex * 3,0,0
//	);
//	++triangleInstanceVal_;
//}
//
//void PrimitiveDrawer::Quad(const Vec3f& p0,
//						   const Vec3f& p1,
//						   const Vec3f& p2,
//						   const Vec3f& p3,
//						   const  IConstantBuffer<Transform>& Transform,
//						   const IConstantBuffer<CameraTransform>& viewProj,
//						   const IConstantBuffer<Material>* material){
//	ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();
//
//	const uint32_t startIndex = quadInstanceVal_ * 6;
//	const uint32_t startVertex = quadInstanceVal_ * 4;
//	quadMesh_->vertData[startVertex].pos = {p0.x,p0.y,p0.z,1.0f};
//	quadMesh_->vertData[startVertex].normal = p0;
//	quadMesh_->vertData[startVertex + 1].pos = {p1.x,p1.y,p1.z,1.0f};
//	quadMesh_->vertData[startVertex + 1].normal = p1;
//	quadMesh_->vertData[startVertex + 2].pos = {p2.x,p2.y,p2.z,1.0f};
//	quadMesh_->vertData[startVertex + 2].normal = p2;
//	quadMesh_->vertData[startVertex + 3].pos = {p3.x,p3.y,p3.z,1.0f};
//	quadMesh_->vertData[startVertex + 3].normal = p3;
//
//	quadMesh_->indexData[startIndex] = startIndex;
//	quadMesh_->indexData[startIndex + 1] = startIndex + 1;
//	quadMesh_->indexData[startIndex + 2] = startIndex + 2;
//	quadMesh_->indexData[startIndex + 3] = startIndex + 1;
//	quadMesh_->indexData[startIndex + 4] = startIndex + 3;
//	quadMesh_->indexData[startIndex + 5] = startIndex + 2;
//
//	commandList->SetGraphicsRootSignature(trianglePso_[(int)currentBlendMode_]->rootSignature.Get());
//	commandList->SetPipelineState(trianglePso_[(int)currentBlendMode_]->pipelineState.Get());
//
//	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	commandList->IASetVertexBuffers(0,1,&quadMesh_->vbView);
//	commandList->IASetIndexBuffer(&quadMesh_->ibView);
//
//	Transform.SetForRootParameter(commandList,0);
//	viewProj.SetForRootParameter(commandList,1);
//	material->SetForRootParameter(commandList,2);
//	Engine::getInstance()->getLightManager()->SetForRootParameter(commandList);
//
//	commandList->DrawIndexedInstanced(
//		6,1,0,startVertex,0
//	);
//	++quadInstanceVal_;
//}

void PrimitiveDrawer::PreDrawLine() {
    ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(linePso_[(int)currentBlendMode_]->rootSignature.Get());
    commandList->SetPipelineState(linePso_[(int)currentBlendMode_]->pipelineState.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);

    Engine::getInstance()->getLightManager()->SetForRootParameter(commandList);
}

void PrimitiveDrawer::PostDrawLine() {
    ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();

    commandList->IASetVertexBuffers(0, 1, &lineMesh_->vbView);
    commandList->IASetIndexBuffer(&lineMesh_->ibView);
}

void PrimitiveDrawer::CreatePso(Engine* engine) {

    ShaderManager* shaderManager = ShaderManager::getInstance();
    ///=================================================
    /// shader読み込み
    ///=================================================

    shaderManager->LoadShader("Object3d.VS");
    shaderManager->LoadShader("Object3d.PS", shaderDirectory, L"ps_6_0");
    ;

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo primShaderInfo;
    primShaderInfo.vsKey = "Object3d.VS";
    primShaderInfo.psKey = "Object3d.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[7]{};
    rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    // PixelShaderで使う
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    // レジスタ番号0 とバインド
    // register(b0) の 0. b11 なら 11
    rootParameter[0].Descriptor.ShaderRegister = 0;
    primShaderInfo.pushBackRootParameter(rootParameter[0]);

    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 2;
    primShaderInfo.pushBackRootParameter(rootParameter[1]);

    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    primShaderInfo.pushBackRootParameter(rootParameter[2]);

    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1; // t1 register for DirectionalLight StructuredBuffer
    primShaderInfo.pushBackRootParameter(rootParameter[3]);

    // PointLight ... 4 (StructuredBuffer)
    rootParameter[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[4].Descriptor.ShaderRegister = 3; // t3 register for PointLight StructuredBuffer
    primShaderInfo.pushBackRootParameter(rootParameter[4]);

    // SpotLight ... 5 (StructuredBuffer)
    rootParameter[5].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[5].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[5].Descriptor.ShaderRegister = 4; // t4 register for SpotLight StructuredBuffer
    primShaderInfo.pushBackRootParameter(rootParameter[5]);
    // lightCounts
    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[6].Descriptor.ShaderRegister = 5;
    primShaderInfo.pushBackRootParameter(rootParameter[6]);

    D3D12_DESCRIPTOR_RANGE directionalLightRange[1]            = {};
    directionalLightRange[0].BaseShaderRegister                = 1;
    directionalLightRange[0].NumDescriptors                    = 1;
    directionalLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    directionalLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE pointLightRange[1]            = {};
    pointLightRange[0].BaseShaderRegister                = 3;
    pointLightRange[0].NumDescriptors                    = 1;
    pointLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pointLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE spotLightRange[1]            = {};
    spotLightRange[0].BaseShaderRegister                = 4;
    spotLightRange[0].NumDescriptors                    = 1;
    spotLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    spotLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    primShaderInfo.SetDescriptorRange2Parameter(directionalLightRange, 1, 3);
    primShaderInfo.SetDescriptorRange2Parameter(pointLightRange, 1, 4);
    primShaderInfo.SetDescriptorRange2Parameter(spotLightRange, 1, 5);
#pragma endregion

#pragma region "Input Element"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION";                     /*Semantics*/
    inputElementDesc.SemanticIndex            = 0;                              /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; //float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    primShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    primShaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

    ///=================================================
    /// BlendMode ごとの Pso作成
    ///=================================================
    /*for(size_t i = 0; i < kBlendNum; ++i){
		primShaderInfo.blendMode_=static_cast<BlendMode>(i);
		trianglePso_[i]=shaderManager->CreatePso(trianglePsoKeys_[i],primShaderInfo,engine->getDxDevice()->getDevice());
	}*/

    /// line
    primShaderInfo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    for (size_t i = 0; i < kBlendNum; ++i) {
        primShaderInfo.blendMode_ = static_cast<BlendMode>(i);
        linePso_[i]               = shaderManager->CreatePso(linePsoKeys_[i], primShaderInfo, engine->getDxDevice()->getDevice());
    }
}
