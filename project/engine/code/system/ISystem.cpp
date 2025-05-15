#include "ISystem.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void ISystem::Update() {
#ifdef _DEBUG
    // 計測開始
    deltaTimer_.Initialize();
#endif

    if (entities_.empty()) {
        return;
    }

    eraseDeadEntity();

    for (auto& entity : entities_) {
        UpdateEntity(entity);
    }

#ifdef _DEBUG
    // 計測終了
    deltaTimer_.Update();
    runningTime_ = deltaTimer_.getDeltaTime();
#endif
}

void ISystem::Edit() {
#ifdef _DEBUG
    // GUI表示
    ImGui::Separator();
    ImGui::Text("SystemType: %s", SystemTypeString[static_cast<int>(systemType_)].c_str());

    ImGui::SetNextItemWidth(78);
    InputGuiCommand("Priority", priority_, "%d");

    ImGui::Text("EntityCount: %d", static_cast<int>(entities_.size()));

    ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;

    if (ImGui::TreeNode("Entities")) {
        if (ImGui::BeginTable("Entities", 2, tableFlags)) {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Type");
            ImGui::TableHeadersRow();

            for (auto& entity : entities_) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", entity->getID());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", entity->getDataType().c_str());
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

#endif
}
