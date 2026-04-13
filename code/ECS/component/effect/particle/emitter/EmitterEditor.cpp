#ifdef _DEBUG

#include "EmitterEditor.h"

#include "Emitter.h"
#include "EmitterShape.h"

#include "editor/EditorController.h"
#include "myGui/MyGui.h"
#include "scene/Scene.h"
#include "component/transform/Transform.h"
#include "entity/Entity.h"

using namespace OriGine;

bool EmitterEditor::Draw(Emitter& _ctrl, const std::string& _parentLabel, Scene* _scene) {
    bool playStartCalled = false;

    // ── Parent Transform ──────────────────────────────────────────────
    if (_scene) {
        // undo/redo 後もキャッシュを最新に保つ
        _ctrl.ResolveParent(_scene);

        ImGui::SeparatorText("Parent Transform");

        auto* transformArray = _scene->GetComponentArray<Transform>();

        // 現在の親ラベルを解決
        std::string currentLabel = "(none)";
        if (_ctrl.parentHandle_.IsValid() && transformArray) {
            for (auto& slot : transformArray->GetSlots()) {
                for (auto& transform : slot.components) {
                    if (transform.GetHandle() == _ctrl.parentHandle_) {
                        auto* entity = _scene->GetEntity(slot.owner);
                        currentLabel = entity ? entity->GetUniqueID() : uuids::to_string(transform.GetHandle().uuid);
                        break;
                    }
                }
            }
        }

        std::string comboLabel = "##ParentTransform" + _parentLabel;
        if (ImGui::BeginCombo(comboLabel.c_str(), currentLabel.c_str())) {
            // (none) 選択
            if (ImGui::Selectable("(none)", !_ctrl.parentHandle_.IsValid())) {
                auto command = std::make_unique<SetterCommand<ComponentHandle>>(
                    &_ctrl.parentHandle_,
                    ComponentHandle{},
                    [&_ctrl, _scene](ComponentHandle*) { _ctrl.ResolveParent(_scene); });
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }
            if (!_ctrl.parentHandle_.IsValid()) {
                ImGui::SetItemDefaultFocus();
            }

            if (transformArray) {
                for (auto& slot : transformArray->GetSlots()) {
                    for (auto& transform : slot.components) {
                        auto* entity  = _scene->GetEntity(slot.owner);
                        std::string itemLabel = entity ? entity->GetUniqueID() : uuids::to_string(transform.GetHandle().uuid);
                        bool selected = (_ctrl.parentHandle_ == transform.GetHandle());
                        if (ImGui::Selectable(itemLabel.c_str(), selected)) {
                            auto newHandle = transform.GetHandle();
                            auto command   = std::make_unique<SetterCommand<ComponentHandle>>(
                                &_ctrl.parentHandle_,
                                newHandle,
                                [&_ctrl, _scene](ComponentHandle*) { _ctrl.ResolveParent(_scene); });
                            OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                        }
                        if (selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();
    }

    CheckBoxCommand("IsActive##" + _parentLabel,           _ctrl.isActive_);
    CheckBoxCommand("IsLoop##" + _parentLabel,             _ctrl.isLoop_);
    CheckBoxCommand("InterpolateSpawnPos##" + _parentLabel, _ctrl.interpolateSpawnPos_);

    std::string label = "Play##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        _ctrl.PlayStart();
        playStartCalled = true;
    }
    ImGui::SameLine();
    label = "Stop##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        _ctrl.PlayStop();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Spawn Settings");
    ImGui::Spacing();

    ImGui::Text("OriginPos");
    label = "##OriginPos" + _parentLabel;
    DragGuiVectorCommand<3, float>(label, _ctrl.originPos_, 0.01f);

    ImGui::Text("ActiveTime");
    label = "##ActiveTime" + _parentLabel;
    DragGuiCommand(label, _ctrl.activeTime_, 0.1f, 0.f);

    ImGui::Text("SpawnCoolTime");
    label = "##SpawnCoolTime" + _parentLabel;
    DragGuiCommand(label, _ctrl.spawnCoolTime_, 0.01f, 0.001f);

    ImGui::Text("SpawnCount");
    label = "##SpawnCount" + _parentLabel;
    DragGuiCommand(label, _ctrl.spawnCount_, 1, 1, 0, "%d");

    ImGui::Spacing();
    ImGui::SeparatorText("Shape");
    ImGui::Spacing();

    EditShape(_ctrl, _parentLabel);

    return playStartCalled;
}

void EmitterEditor::EditShape(Emitter& _ctrl, const std::string& _parentLabel) {
    std::string label = "ShapeType##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), kEmitterShapeTypeWord[static_cast<int32_t>(_ctrl.shapeType_)].c_str())) {
        for (int32_t i = 0; i < kShapeTypeCount; i++) {
            bool isSelected = (_ctrl.shapeType_ == EmitterShapeType(i));
            if (ImGui::Selectable(kEmitterShapeTypeWord[i].c_str(), isSelected)) {
                auto command = std::make_unique<SetterCommand<EmitterShapeType>>(
                    &_ctrl.shapeType_,
                    EmitterShapeType(i),
                    [&_ctrl](EmitterShapeType* _newType) {
                        switch (*_newType) {
                        case EmitterShapeType::SPHERE:
                            _ctrl.spawnShape_ = std::make_shared<EmitterSphere>();
                            break;
                        case EmitterShapeType::BOX:
                            _ctrl.spawnShape_ = std::make_shared<EmitterBox>();
                            break;
                        case EmitterShapeType::CAPSULE:
                            _ctrl.spawnShape_ = std::make_shared<EmitterCapsule>();
                            break;
                        case EmitterShapeType::CONE:
                            _ctrl.spawnShape_ = std::make_shared<EmitterCone>();
                            break;
                        default:
                            break;
                        }
                    });
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (_ctrl.spawnShape_) {
        _ctrl.spawnShape_->Debug(_parentLabel);
    }
}

#endif // _DEBUG
