#include "SystemInspector.h"

/// editor
#include "editor/EditorController.h"
#include "editor/sceneEditor/SceneEditor.h"

/// externals
#include "myGui/MyGui.h"

#pragma region "SystemInspectorArea"

SystemInspectorArea::SystemInspectorArea(SceneEditorWindow* _window) : Editor::Area("SystemInspectorArea"), parentWindow_(_window) {}
SystemInspectorArea::~SystemInspectorArea() {}

void SystemInspectorArea::Initialize() {
    auto& allSystems = SystemRegistry::getInstance()->getSystemsRef();
    for (auto& [name, system] : allSystems) {
        if (!system) {
            LOG_ERROR("SystemInspectorArea::Initialize: System '{}' is null.", name);
            continue;
        }
        int32_t category = int32_t(system->getCategory());
        systemMap_[category].emplace_back(std::make_pair<>(name, system.get()));
    }

    for (auto& systemByCategory : systemMap_) {
        std::sort(systemByCategory.begin(), systemByCategory.end(), [](const std::pair<std::string, ISystem*>& a, const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority(); // Priority でソート
        });
    }
}

void SystemInspectorArea::DrawGui() {
    bool isOpen = isOpen_.current();
    if (ImGui::Begin(name_.c_str(), &isOpen)) {
        areaSize_ = ImGui::GetContentRegionAvail();
        ImGui::Text("System Inspector Area");
        ImGui::Separator();
        auto currentScene = parentWindow_->getCurrentScene();

        if (!currentScene) {
            ImGui::Text("No current scene found.");
            return;
        }

        if (ImGui::TreeNode("SystemCategoryActivity##SystemInspectorArea")) {
            std::string label = "";
            for (int32_t category = 0; category < static_cast<int32_t>(SystemCategory::Count); ++category) {
                label         = SystemCategoryString[category] + "##SystemCategoryActivity";
                bool activity = currentScene->getSystemRunner()->getCategoryActivity()[category];
                if (ImGui::Checkbox(label.c_str(), &activity)) {
                    auto command = std::make_unique<ChangeSystemCategoryActivity>(this, static_cast<SystemCategory>(category), currentScene->getSystemRunner()->getCategoryActivity()[category], activity);
                    EditorController::getInstance()->pushCommand(std::move(command));
                }
            }
            ImGui::TreePop();
        }

        std::string label = "";
        label             = "Filter##" + currentScene->getName();

        int32_t filter                        = static_cast<int32_t>(filter_);
        constexpr int32_t categoryFilterStart = 3;
        bool activeFilter                     = (filter_ & FilterType::ACTIVE) != 0;
        bool inactiveFilter                   = (filter_ & FilterType::INACTIVE) != 0;

        if (ImGui::TreeNode(label.c_str())) {
            constexpr int32_t categoryMask = ~((1 << (categoryFilterStart + 1)) - 1); // 0b11111100...

            ImGui::Checkbox("Active##SystemInspectorArea::FilterType", &activeFilter);
            ImGui::Checkbox("Inactive##SystemInspectorArea::FilterType", &inactiveFilter);

            if (activeFilter) {
                filter |= int32_t(FilterType::ACTIVE);
            } else {
                filter &= ~int32_t(FilterType::ACTIVE);
            }
            if (inactiveFilter) {
                filter |= int32_t(FilterType::INACTIVE);
            } else {
                filter &= ~int32_t(FilterType::INACTIVE);
            }

            ImGui::Separator();

            if (ImGui::Button("ActiveAllCategory##SystemInspectorArea")) {
                // 全てのカテゴリをチェックする
                filter |= ((1 << static_cast<int32_t>(SystemCategory::Count)) - 1) << categoryFilterStart;
            }
            if (ImGui::Button("InactiveAllCategory##SystemInspectorArea")) {
                // 全てのカテゴリをチェックしない
                filter &= categoryMask; // カテゴリビットをクリア
            }
            // カテゴリチェックボックス
            for (size_t i = 0; i < static_cast<size_t>(SystemCategory::Count); ++i) {
                label             = SystemCategoryString[i];
                int32_t filterBit = (1 << (i + categoryFilterStart));
                bool isChecked    = (filter & filterBit) != 0;
                if (ImGui::Checkbox(label.c_str(), &isChecked)) {
                    if (isChecked) {
                        filter |= filterBit;
                    } else {
                        filter &= ~filterBit;
                    }
                }
            }
            ImGui::TreePop();
        }

        {
            label = "SearchSystems##SystemInspectorArea";
            ImGui::InputText(label.c_str(), &searchBuffer_[0], sizeof(char) * searchBufferSize_);
            static GuiValuePool<std::string> entityNamePool;
            if (ImGui::IsItemActive()) {
                entityNamePool.setValue(label, searchBuffer_);
            } else if (ImGui::IsItemDeactivatedAfterEdit()) {
                auto command = std::make_unique<ChangeSearchFilter>(this, entityNamePool.popValue(label));
                EditorController::getInstance()->pushCommand(std::move(command));
            }
        }

        filter = searchBuffer_.empty() ? filter : static_cast<int32_t>(FilterType::SEARCH);

        if (filter_ != filter) {
            filter_      = static_cast<FilterType>(filter);
            auto command = std::make_unique<ChangeSystemFilter>(this, static_cast<int32_t>(filter_));
            EditorController::getInstance()->pushCommand(std::move(command));
        }

        bool searchFilter = (filter_ & FilterType::SEARCH) != 0;
        for (size_t i = 0; i < systemMap_.size(); ++i) {
            if (searchFilter) {
                for (const auto& [systemName, system] : systemMap_[i]) {
                    if (!system) {
                        LOG_ERROR("SystemInspectorArea::DrawGui: System '{}' is null.", systemName);
                        continue;
                    }
                    if (systemName.find(searchBuffer_) != std::string::npos) {
                        if (activeFilter && !system->isActive()) {
                            continue; // アクティブフィルタに合致しない場合はスキップ
                        } else if (inactiveFilter && system->isActive()) {
                            continue; // 非アクティブフィルタに合致しない場合はスキップ
                        }
                        SystemGui(systemName, system);
                    }
                }

            } else {
                int32_t filterBit = (1 << (i + categoryFilterStart));
                if ((filter_ & filterBit) == 0) {
                    continue; // フィルタリングされている場合はスキップ
                }
                if (ImGui::CollapsingHeader(SystemCategoryString[i].c_str())) {

                    ImGui::Indent();
                    for (auto& [systemName, system] : systemMap_[i]) {
                        if (!system) {
                            LOG_ERROR("SystemInspectorArea::DrawGui: System '{}' is null.", systemName);
                            continue;
                        }
                        if (activeFilter) {
                            if (!system->isActive()) {
                                continue; // アクティブフィルタに合致しない場合はスキップ
                            }
                        } else if (inactiveFilter) {
                            if (system->isActive()) {
                                continue; // 非アクティブフィルタに合致しない場合はスキップ
                            }
                        }
                        SystemGui(systemName, system);
                    }
                    ImGui::Unindent();
                }
            }
        }
    }

    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}

void SystemInspectorArea::Finalize() {
    Editor::Area::Finalize();
}

void SystemInspectorArea::SystemGui(const std::string& _systemName, ISystem* _system) {
    ImGui::PushID(_systemName.c_str());
    bool isActive = _system->isActive();

    if (ImGui::Checkbox("##Active", &isActive)) {
        auto command = std::make_unique<ChangeSystemActivity>(this, _systemName, _system->isActive(), isActive);
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    ImGui::SameLine();

    constexpr int32_t inputPriorityBoxWidth = 76;
    ImGui::SetNextItemWidth(inputPriorityBoxWidth);
    int32_t priority = _system->getPriority();
    if (ImGui::InputInt("##Priority", &priority, 1)) {
        auto command = std::make_unique<ChangeSystemPriority>(this, _systemName, _system->getPriority(), priority);
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    ImGui::SameLine();

    ImGui::Text("%s", _systemName.c_str());

    ImGui::PopID();
}

#pragma region "Commands"

SystemInspectorArea::ChangeSystemPriority::ChangeSystemPriority(SystemInspectorArea* _inspectorArea, const std::string& _systemName, int32_t _oldPriority, int32_t _newPriority)
    : inspectorArea_(_inspectorArea), systemName_(_systemName), oldPriority_(_oldPriority), newPriority_(_newPriority) {}
void SystemInspectorArea::ChangeSystemPriority::Execute() {
    SystemRegistry* systemRegistry = SystemRegistry::getInstance();

    // exe 内に Systemが 登録されているか確認
    auto systemItr = systemRegistry->getSystemsRef().find(systemName_);
    if (systemItr == systemRegistry->getSystemsRef().end()) {
        LOG_ERROR("ChangeSystemPriority::Execute: System '{}' not found.", systemName_);
        return;
    }

    systemItr->second->setPriority(newPriority_);

    int32_t categoryIndex   = static_cast<int32_t>(systemItr->second->getCategory());
    auto* systemsByCategory = &inspectorArea_->systemMap_[categoryIndex];
    std::sort(systemsByCategory->begin(),
        systemsByCategory->end(),
        [](const std::pair<std::string, ISystem*>& a, const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority(); // Priority でソート
        });
}
void SystemInspectorArea::ChangeSystemPriority::Undo() {
    SystemRegistry* systemRegistry = SystemRegistry::getInstance();

    // exe 内に Systemが 登録されているか確認
    auto systemItr = systemRegistry->getSystemsRef().find(systemName_);
    if (systemItr == systemRegistry->getSystemsRef().end()) {
        LOG_ERROR("ChangeSystemPriority::Execute: System '{}' not found.", systemName_);
        return;
    }

    systemItr->second->setPriority(oldPriority_);

    int32_t categoryIndex   = static_cast<int32_t>(systemItr->second->getCategory());
    auto* systemsByCategory = &inspectorArea_->systemMap_[categoryIndex];
    std::sort(systemsByCategory->begin(),
        systemsByCategory->end(),
        [](const std::pair<std::string, ISystem*>& a, const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority(); // Priority でソート
        });
}

SystemInspectorArea::ChangeSystemActivity::ChangeSystemActivity(
    SystemInspectorArea* _inspectorArea,
    const std::string& _systemName,
    bool _oldActivity,
    bool _newActivity)
    : inspectorArea_(_inspectorArea),
      systemName_(_systemName),
      oldActivity_(_oldActivity),
      newActivity_(_newActivity) {}
void SystemInspectorArea::ChangeSystemActivity::Execute() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemActivity::Execute: No current scene found.");
        return;
    }
    auto* targetSystem = SystemRegistry::getInstance()->getSystem(systemName_);
    if (!targetSystem) {
        LOG_ERROR("ChangeSystemActivity::Execute: System '{}' not found in registry.", systemName_);
        return;
    }
    // true にする場合, SystemRegistry から システムを取得
    if (newActivity_) {
        currentScene->registerSystem(systemName_, targetSystem->getPriority());
    } else {
        currentScene->unregisterSystem(systemName_);
    }
}
void SystemInspectorArea::ChangeSystemActivity::Undo() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemActivity::Undo: No current scene found.");
        return;
    }
    // false にする場合, SystemRegistry から システムを取得
    if (oldActivity_) {
        currentScene->registerSystem(systemName_);
    } else {
        currentScene->unregisterSystem(systemName_);
    }
}

void SystemInspectorArea::ChangeSystemFilter::Execute() {
    inspectorArea_->filter_ = newFilter_;
    LOG_DEBUG("ChangeSystemFilter::Execute: Changed system filter to {}.", newFilter_);
}

void SystemInspectorArea::ChangeSystemFilter::Undo() {
    inspectorArea_->filter_ = oldFilter_;
    LOG_DEBUG("ChangeSystemFilter::Undo: Reverted system filter to {}.", oldFilter_);
}

void SystemInspectorArea::ChangeSearchFilter::Execute() {
    inspectorArea_->searchBuffer_ = newSearchBuffer_;
    LOG_DEBUG("ChangeSearchFilter::Execute: Changed search filter to '{}'.", newSearchBuffer_);
}

void SystemInspectorArea::ChangeSearchFilter::Undo() {
    inspectorArea_->searchBuffer_ = oldSearchBuffer_;
    LOG_DEBUG("ChangeSearchFilter::Undo: Reverted search filter to '{}'.", oldSearchBuffer_);
}

SystemInspectorArea::ChangeSystemCategoryActivity::ChangeSystemCategoryActivity(SystemInspectorArea* _inspectorArea, SystemCategory _category, bool _oldActivity, bool _newActivity)
    : inspectorArea_(_inspectorArea), category_(_category), oldActivity_(_oldActivity), newActivity_(_newActivity) {}
SystemInspectorArea::ChangeSystemCategoryActivity::~ChangeSystemCategoryActivity() {}

void SystemInspectorArea::ChangeSystemCategoryActivity::Execute() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemCategoryActivity::Execute: No current scene found.");
        return;
    }
    auto* systemRunner = currentScene->getSystemRunnerRef();
    if (!systemRunner) {
        LOG_ERROR("ChangeSystemCategoryActivity::Execute: No SystemRunner found in current scene.");
        return;
    }
    systemRunner->setCategoryActivity(category_, newActivity_);
}

void SystemInspectorArea::ChangeSystemCategoryActivity::Undo() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemCategoryActivity::Undo: No current scene found.");
        return;
    }
    auto* systemRunner = currentScene->getSystemRunnerRef();
    if (!systemRunner) {
        LOG_ERROR("ChangeSystemCategoryActivity::Undo: No SystemRunner found in current scene.");
        return;
    }
    systemRunner->setCategoryActivity(category_, oldActivity_);
}

#pragma endregion

#pragma endregion
