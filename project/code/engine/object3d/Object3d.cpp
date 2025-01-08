#include "object3d/Object3d.h"

#include "model/Model.h"
#include "model/ModelManager.h"

#include <cassert>
#include <unordered_map>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include "Engine.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "camera/CameraManager.h"
#include "directX12/DxFunctionHelper.h"
#include "directX12/DxHeap.h"
#include "material/Material.h"
#include "material/texture/TextureManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"

#include <stdint.h>

BlendMode Object3d::currentBlend_ = BlendMode::Alpha;

#pragma region "Object3d"
void Object3d::PreDraw() {
    currentBlend_ = BlendMode::Alpha;

    ModelManager* manager = ModelManager::getInstance();
    auto* commandList     = manager->dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(manager->texturePso_[static_cast<uint32_t>(currentBlend_)]->rootSignature.Get());
    commandList->SetPipelineState(manager->texturePso_[static_cast<uint32_t>(currentBlend_)]->pipelineState.Get());

    Engine::getInstance()->getLightManager()->SetForRootParameter(commandList);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
}

void Object3d::setBlendMode(BlendMode blend) {
    currentBlend_         = blend;
    ModelManager* manager = ModelManager::getInstance();
    auto* commandList     = manager->dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(manager->texturePso_[static_cast<uint32_t>(currentBlend_)]->rootSignature.Get());
    commandList->SetPipelineState(manager->texturePso_[static_cast<uint32_t>(currentBlend_)]->pipelineState.Get());
}

Object3d::Object3d() {}

Object3d::~Object3d() {}

void Object3d::Init(const std::string& directoryPath, const std::string& filename) {
    data_ = ModelManager::getInstance()->Create(
        directoryPath,
        filename,
        [this](Model* model) {
            transform_.Init();
            transform_.UpdateMatrix();
            for (auto& mesh : model->meshData_->mesh_) {
                model->transformBuff_[&mesh].openData_.parent = &transform_;
                model->transformBuff_[&mesh].openData_.UpdateMatrix();
                model->transformBuff_[&mesh].ConvertToBuffer();
            }
        });
}

void Object3d::DrawThis() {
    ModelManager* manager = ModelManager::getInstance();
    auto* commandList     = manager->dxCommand_->getCommandList();

    uint32_t index = 0;

    for (auto& mesh : data_->meshData_->mesh_) {
        auto& material = data_->materialData_[index];

        IConstantBuffer<Transform>& meshTransform = data_->transformBuff_[&mesh];
        meshTransform.ConvertToBuffer();

        ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
        commandList->SetDescriptorHeaps(1, ppHeaps);
        commandList->SetGraphicsRootDescriptorTable(
            7,
            TextureManager::getDescriptorGpuHandle(material.textureNumber));

        commandList->IASetVertexBuffers(0, 1, &mesh.meshBuff->vbView);
        commandList->IASetIndexBuffer(&mesh.meshBuff->ibView);

        meshTransform.SetForRootParameter(commandList, 0);

        material.material->SetForRootParameter(commandList, 2);
        // 描画!!!
        commandList->DrawIndexedInstanced(UINT(mesh.indexSize), 1, 0, 0, 0);

        ++index;
    }
}

void Object3d::setMaterial(IConstantBuffer<Material>* material, uint32_t index) {
    data_->materialData_[index].material = material;
}

void Object3d::UpdateTransform() {
    transform_.UpdateMatrix();
    for (auto& mesh : data_->meshData_->mesh_) {
        data_->transformBuff_[&mesh].openData_.parent = &transform_;
        data_->transformBuff_[&mesh].openData_.UpdateMatrix();
    }
}

void Object3d::Draw() {
    drawFuncTable_[(size_t)data_->meshData_->currentState_]();
}
#pragma endregion
