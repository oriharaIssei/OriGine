#include "Editor.h"

#include "winApp/WinApp.h"

#include "imgui/imgui.h"

Editor* Editor::getInstance(){
	static Editor instance{};
	return &instance;
}

void Editor::Finalize(){
	menuBarFuncs_.clear();
}

void Editor::Begin(const std::string& windowName){
	ImGui::SetNextWindowPos({0.0f,0.0f});
	ImGui::SetNextWindowSize({static_cast<float>(window_->getWidth()),static_cast<float>(window_->getHeight())});
	ImGui::Begin(windowName.c_str(),nullptr,
				 ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
				 ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
				 ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
				 ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar);
	MenuBarUpdate();
}

void Editor::End(){
	ImGui::End();
}

void Editor::MenuBarUpdate(){
	ImGui::BeginMenuBar();
	for(auto& [label,update] : menuBarFuncs_){
		if(ImGui::BeginMenu(label.c_str())){
			update();
			ImGui::EndMenu();
		}
	}
	ImGui::EndMenuBar();
}

void Editor::setWinApp(WinApp* _winApp){
	window_ = _winApp;
}