#include "SystemInspector.h"

#ifdef _DEBUG

/// ECS
// system
#include "system/SystemRegistry.h"
#include "system/SystemRunner.h"

/// editor
#include "editor/EditorController.h"
#include "editor/sceneEditor/SceneEditor.h"

/// externals
#include "myGui/MyGui.h"

#pragma region "SystemInspectorArea"

SystemInspectorArea::SystemInspectorArea(SceneEditorWindow* _window) : Editor::Area("SystemInspectorArea"), parentWindow_(_window) {}
SystemInspectorArea::~SystemInspectorArea() {}

void SystemInspectorArea::Initialize() {
    auto scene       = parentWindow_->getCurrentScene();
    auto& allSystems = SystemRegistry::getInstance()->getSystemsRef();
    for (auto& [name, createSystemFunc] : allSystems) {
        auto createdScene = createSystemFunc(scene);
        int32_t category  = int32_t(createdScene->getCategory());
        systemMap_[category].emplace_back(std::make_pair<>(name, 0));
    }

    for (auto& systemByCategory : systemMap_) {
        std::sort(systemByCategory.begin(), systemByCategory.end(), [](const std::pair<std::string, int32_t>& a, const std::pair<std::string, int32_t>& b) {
            return a.second < b.second; // Priority でソート
        });
    }
}

void SystemInspectorArea::DrawGui() {
    bool isOpen = isOpen_.current();
    if (!isOpen) {
        return;
    }
    if (ImGui::Begin(name_.c_str(), &isOpen)) {

        areaSize_ = ImGui::GetContentRegionAvail();
        ImGui::Text("System Inspector Area");
        ImGui::Separator();
        auto currentScene = parentWindow_->getCurrentScene();

        if (!currentScene) {
            ImGui::Text("No current scene found.");
            ImGui::End();
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
        constexpr int32_t categoryFilterStart = int32_t(FilterType::CATEGORY_INITIALIZE) - 1;
        if (ImGui::TreeNode(label.c_str())) {
            constexpr int32_t categoryMask = ~((1 << (categoryFilterStart + 1)) - 1); // 0b11111100...

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
        for (int32_t i = 0; i < systemMap_.size(); ++i) {
            if (searchFilter) {
                for (auto& [systemName, priority] : systemMap_[i]) {
                    if (systemName.find(searchBuffer_) != std::string::npos) {
                        SystemGui(systemName, priority);
                    }
                }

            } else {
                int32_t filterBit = (1 << (i + categoryFilterStart));
                if ((filter_ & filterBit) == 0) {
                    continue; // フィルタリングされている場合はスキップ
                }
                if (ImGui::CollapsingHeader(SystemCategoryString[i].c_str())) {

                    ImGui::Indent();
                    for (auto& [systemName, priority] : systemMap_[i]) {
                        SystemGui(systemName, priority);
                    }
                    ImGui::Unindent();
                }
            }
        }

        for (auto& systemByCategory : systemMap_) {
            std::sort(systemByCategory.begin(), systemByCategory.end(), [](const std::pair<std::string, int32_t>& a, const std::pair<std::string, int32_t>& b) {
                return a.second < b.second; // Priority でソート
            });
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

void SystemInspectorArea::SystemGui(const std::string& _systemName, int32_t& _priority) {
    auto currentScene = parentWindow_->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("SystemInspectorArea::SystemGui: No current scene found.");
        return;
    }

    ImGui::PushID(_systemName.c_str());

    auto* system = currentScene->getSystem(_systemName);
    if (system && system->isActive()) {
        bool isActive = system->isActive();

        if (ImGui::Checkbox("##Active", &isActive)) {
            auto command = std::make_unique<ChangeSystemActivity>(this, _systemName, system->getPriority(), system->isActive(), isActive);
            EditorController::getInstance()->pushCommand(std::move(command));
        }

        ImGui::SameLine();

        constexpr int32_t inputPriorityBoxWidth = 76;
        ImGui::SetNextItemWidth(inputPriorityBoxWidth);
        int32_t priority = system->getPriority();
        if (ImGui::InputInt("##Priority", &priority, 1)) {
            auto command = std::make_unique<ChangeSystemPriority>(this, _systemName, system->getPriority(), priority);
            EditorController::getInstance()->pushCommand(std::move(command));
        }

        _priority = priority; // 更新された優先度をアイテムに反映

        ImGui::SameLine();

        ImGui::Text("%s", _systemName.c_str());

    } else {
        bool isActive = false;
        _priority     = 9999;
        if (ImGui::Checkbox("##Active", &isActive)) {
            currentScene->registerSystem(_systemName, 0); // 0 はデフォルトの優先度
            auto command = std::make_unique<ChangeSystemActivity>(this, _systemName, 0, false, true);
            EditorController::getInstance()->pushCommand(std::move(command));

            ImGui::PopID();
            return;
        }

        ImGui::SameLine();

        ImGui::Text("%s", _systemName.c_str());
    };

    ImGui::PopID();
}

#pragma region "Commands"

SystemInspectorArea::ChangeSystemPriority::ChangeSystemPriority(SystemInspectorArea* _inspectorArea, const std::string& _systemName, int32_t _oldPriority, int32_t _newPriority)
    : inspectorArea_(_inspectorArea), systemName_(_systemName), oldPriority_(_oldPriority), newPriority_(_newPriority) {}

void SystemInspectorArea::ChangeSystemPriority::Execute() {
    auto* systemRunner = inspectorArea_->getParentWindow()->getCurrentScene()->getSystemRunner();

    // exe 内に Systemが 登録されているか確認
    auto systemItr = systemRunner->getSystems().find(systemName_);
    if (systemItr == systemRunner->getSystems().end()) {
        LOG_ERROR("ChangeSystemPriority::Execute: System '{}' not found.", systemName_);
        return;
    }

    systemItr->second->setPriority(newPriority_);

    int32_t categoryIndex   = static_cast<int32_t>(systemItr->second->getCategory());
    auto* systemsByCategory = &inspectorArea_->systemMap_[categoryIndex];

    std::sort(systemsByCategory->begin(),
        systemsByCategory->end(),
        [](const std::pair<std::string, int32_t>& a, const std::pair<std::string, int32_t>& b) {
            return a.second < b.second; // Priority でソート
        });
}

void SystemInspectorArea::ChangeSystemPriority::Undo() {
    auto* systemRunner = inspectorArea_->getParentWindow()->getCurrentScene()->getSystemRunner();

    // exe 内に Systemが 登録されているか確認
    auto systemItr = systemRunner->getSystems().find(systemName_);
    if (systemItr == systemRunner->getSystems().end()) {
        LOG_ERROR("ChangeSystemPriority::Execute: System '{}' not found.", systemName_);
        return;
    }

    systemItr->second->setPriority(oldPriority_);

    int32_t categoryIndex   = static_cast<int32_t>(systemItr->second->getCategory());
    auto* systemsByCategory = &inspectorArea_->systemMap_[categoryIndex];
    std::sort(systemsByCategory->begin(),
        systemsByCategory->end(),
        [](const std::pair<std::string, int32_t>& a, const std::pair<std::string, int32_t>& b) {
            return a.second < b.second; // Priority でソート
        });
}

SystemInspectorArea::ChangeSystemActivity::ChangeSystemActivity(
    SystemInspectorArea* _inspectorArea,
    const std::string& _systemName,
    int32_t _systemPriority,
    bool _oldActivity,
    bool _newActivity)
    : inspectorArea_(_inspectorArea),
      systemName_(_systemName),
      systemPriority_(_systemPriority),
      oldActivity_(_oldActivity),
      newActivity_(_newActivity) {}

void SystemInspectorArea::ChangeSystemActivity::Execute() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemActivity::Execute: No current scene found.");
        return;
    }
    // true にする場合, SystemRegistry から システムを取得
    if (newActivity_) {
        currentScene->registerSystem(systemName_, systemPriority_);
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
        currentScene->registerSystem(systemName_, systemPriority_);
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

#endif // _DEBUG
