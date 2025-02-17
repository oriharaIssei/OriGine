#include "EngineEditor.h"

#ifdef _DEBUG
///engine
//module
#include "scene/SceneManager.h"
#include "camera/CameraManager.h"
#include "material/Material.h"
#include "material/light/LightManager.h"
//scene
#include "scene/IScene.h"
//externals
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

EngineEditor::EngineEditor(){
    // Constructor implementation
}

EngineEditor::~EngineEditor(){
    // Destructor implementation
}

void EngineEditor::Update(){
    // MainMenuBar
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("Scene")){
            SceneManager* sceneManager = SceneManager::getInstance();
            for(auto& [name,index] : sceneManager->sceneIndexs_){
                if(ImGui::MenuItem(name.c_str())){
                    sceneManager->changeScene(name);
                    break;
                }
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Editors")){
            if(ImGui::BeginMenu("ActiveState")){
                for(auto& [name,editor] : editors_){
                    ImGui::Checkbox(name.c_str(),&editorActive_[editor.get()]);
                }
                ImGui::EndMenu();
            }
            if(ImGui::MenuItem("IsDebug")){
                ImGui::Checkbox("IsDebug",&isActive_);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if(!isActive_){
        return;
    }
    for(auto& [name,editor] : editors_){
        if(editorActive_[editor.get()]){
            editor->Update();
        }
    }
}

#pragma region "アクセッサー"
void EngineEditor::addEditor(const std::string& name,std::unique_ptr<IEditor>&& editor){
    editors_[name]                      = std::move(editor);
    editorActive_[editors_[name].get()] = false;
}

#pragma endregion

#endif // _DEBUG
