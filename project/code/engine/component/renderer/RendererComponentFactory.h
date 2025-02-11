#pragma once

// interface
#include "IRendererComponent.h"

/// stl
#include <concepts>
#include <memory>
#include <string>

/// engine
//assets
#include "model/Model.h"
// component
#include "MeshRender.h"
// utility
#include "util/TypeName.h"
// manager
#include "model/ModelManager.h"
#include "RenderManager.h"

///==================================================================================================================
// ファクトリ関数の定義
///==================================================================================================================

template <IsDerivedIRenderComponent RendererComponentTemplate, IsDerivedRendererController RendererComponentControllerTemplate = RendererComponentController<RendererComponentTemplate>, typename... Args>
inline std::shared_ptr<RendererComponentTemplate> CreateRendererComponent(Args&&... _args) {
    // RenderManager から対応する Controller を取得
    RendererComponentControllerTemplate* controller =
        RenderManager::getInstance()->getRendererController<RendererComponentControllerTemplate>();
    if (!controller) {
        // Controller が存在しない場合は作成
        auto controllerInstance = std::make_unique<RendererComponentControllerTemplate>();
        controllerInstance->Init();
        RenderManager::getInstance()->addMeshController(std::move(controllerInstance));
        controller = RenderManager::getInstance()->getRendererController<RendererComponentControllerTemplate>();
    }

    // component 作成・初期化
    std::shared_ptr<RendererComponentTemplate> component = std::make_shared<RendererComponentTemplate>(std::forward<Args>(_args)...);
    component->Init();

    // Controller に component を追加
    controller->addRenderer(component);
    return component;
}

std::shared_ptr<TextureMeshRenderer> CreateFromModelFile(const std::string& _directory, const std::string& _filename) {
    // 既存のmapから、rootNodeに沿った順序(親→子)でMeshGroupをvectorへ変換
    std::vector<TextureMesh> orderedMeshGroup;
    std::vector<Transform> orderedTransform;
    std::vector<TexturedMaterial> orderedMaterial;

    std::function<void(Model*, const ModelNode&)>
        traverse = [&](Model* _model, const ModelNode& node) {
            auto it = _model->meshData_->meshGroup_.find(node.name);
            if (it != _model->meshData_->meshGroup_.end()) {
                orderedMeshGroup.push_back(it->second);
                orderedMeshGroup.back().setName(node.name);
                orderedTransform.push_back(_model->transforms_[&it->second]);
                orderedMaterial.push_back(_model->materialData_[orderedMeshGroup.size() - 1]);
            }
            for (const auto& child : node.children) {
                traverse(_model, child);
            }
        };
    auto model = ModelManager::getInstance()->Create(_directory, _filename, [&](Model* model) {
        if (model == nullptr) {
            return;
        }
        traverse(model, model->meshData_->rootNode);
    });
    if (model == nullptr) {
        return nullptr;
    }

    // renderer作成時にorderedMeshGroupをセット
    std::shared_ptr<TextureMeshRenderer> renderer = CreateRendererComponent<TextureMeshRenderer, TextureMeshRendererController>(orderedMeshGroup);
    for (int i = 0; i < orderedMeshGroup.size(); ++i) {
        renderer->setTransform(i, orderedTransform[i]);
        renderer->setMaterialBuff(i, orderedMaterial[i].material);
        renderer->setTextureNumber(i, orderedMaterial[i].textureNumber);
    }
    return renderer;
}
