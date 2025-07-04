#include "SkinningAnimationComponent.h"

/// stl
#include <algorithm>

#define RESOURCE_DIRECTORY
/// engine
#include "AnimationManager.h"
#include "EngineInclude.h"

/// ECS
// component
#include "component/renderer/MeshRenderer.h"
#include "ECS/ECSManager.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void SkinningAnimationComponent::Initialize(GameEntity* _entity) {
    entity_ = _entity;

    if (!directory_.empty() && !fileName_.empty()) {
        Load(directory_, fileName_);
    }
    currentTime_ = 0;
}

bool SkinningAnimationComponent::Edit() {
    bool isChanged = false;

#ifdef _DEBUG
    ImGui::Text("Animation File: %s", fileName_.c_str());
    if (ImGui::Button("Load")) {
        std::string directory;
        std::string fileName;
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"gltf", "anm"})) {
            auto setPath = std::make_unique<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory);
            auto setFile = std::make_unique<SetterCommand<std::string>>(&fileName_, fileName);
            CommandCombo commandCombo;
            commandCombo.addCommand(std::move(setPath));
            commandCombo.addCommand(std::move(setFile));
            commandCombo.setFuncOnAfterCommand([this]() {
                animationData_ = AnimationManager::getInstance()->Load(directory_, fileName_);
                duration_      = animationData_->duration;
            },
                true);
            EditorController::getInstance()->pushCommand(std::make_unique<CommandCombo>(commandCombo));
            isChanged = true;
        }
    }

    if (animationData_) {
        isChanged |= DragGuiCommand("Duration", duration_, 0.01f, 0.0f, 100.0f);

        isChanged |= CheckBoxCommand("Play", animationState_.isPlay_);
        isChanged |= CheckBoxCommand("Loop", animationState_.isLoop_);
        isChanged |= DragGuiCommand("Playback Speed", playbackSpeed_, 0.01f, 0.0f);
    }

    int32_t entityModelMeshRendererSize = ECSManager::getInstance()->getComponentArray<ModelMeshRenderer>()->getComponentSize(entity_);
    InputGuiCommand<int32_t>("Bind Mode MeshRenderer Index", bindModeMeshRendererIndex_, "%d",
        [entityModelMeshRendererSize](int32_t* _newVal) {
            *_newVal = std::clamp(*_newVal, 0, entityModelMeshRendererSize - 1);
        });

#endif // _DEBUG

    return isChanged;
}

void SkinningAnimationComponent::Finalize() {
    animationData_.reset();
    entity_ = nullptr;
}

void SkinningAnimationComponent::Load(const std::string& directory, const std::string& fileName) {
    directory_     = directory;
    fileName_      = fileName;
    animationData_ = AnimationManager::getInstance()->Load(directory_, fileName_);
    if (animationData_) {
        duration_ = animationData_->duration;
    }
}

void SkinningAnimationComponent::Play() {
    if (animationData_) {
        animationState_.isPlay_ = true;
        animationState_.isEnd_  = false;
        currentTime_            = 0.0f;
    }
}

void SkinningAnimationComponent::Stop() {
    if (animationData_) {
        animationState_.isPlay_ = false;
        animationState_.isEnd_  = true;
    }
}

void to_json(nlohmann::json& j, const SkinningAnimationComponent& r) {
    j["directory"] = r.directory_;
    j["fileName"]  = r.fileName_;

    j["bindModeMeshRendererIndex"] = r.bindModeMeshRendererIndex_;

    j["duration"] = r.duration_;

    j["playbackSpeed"] = r.playbackSpeed_;
    j["isPlay"]        = r.animationState_.isPlay_;
    j["isLoop"]        = r.animationState_.isLoop_;
}

void from_json(const nlohmann::json& j, SkinningAnimationComponent& r) {
    j.at("directory").get_to(r.directory_);
    j.at("fileName").get_to(r.fileName_);

    j.at("bindModeMeshRendererIndex").get_to(r.bindModeMeshRendererIndex_);

    j.at("duration").get_to(r.duration_);

    j.at("playbackSpeed").get_to(r.playbackSpeed_);
    j.at("isPlay").get_to(r.animationState_.isPlay_);
    j.at("isLoop").get_to(r.animationState_.isLoop_);
}
