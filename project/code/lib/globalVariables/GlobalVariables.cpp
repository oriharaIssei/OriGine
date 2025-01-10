#include "GlobalVariables.h"

#include <vector>

#include <filesystem>
#include <fstream>

#include <Windows.h>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <nlohmann/json.hpp>

using json = nlohmann::json;
const std::string kDirectoryPath = "./resource/GlobalVariables/";

GlobalVariables* GlobalVariables::getInstance(){
	static GlobalVariables instance;
	return &instance;
}

GlobalVariables::GlobalVariables(){}

GlobalVariables::~GlobalVariables(){}

void GlobalVariables::Update(){
#ifdef _DEBUG
	if(ImGui::Begin("Global Variables",nullptr,ImGuiWindowFlags_MenuBar)){

		ImGuiMenu();

		if(data_[currentScene_].empty()){
			ImGui::End();
			return;
		}

		std::vector<const char*> groupList;  // 動的なグループリストを
		for(auto& group : data_[currentScene_]){
			groupList.push_back(group.first.c_str());
		}

		// ImGui の Combo ボックスでグループリストを表示
		ImGui::Combo("GroupList",&currentGroupNum_,groupList.data(),static_cast<int>(groupList.size()));
		currentGroup_ = &data_[currentScene_][groupList[currentGroupNum_]];
		currentGroupName_ = groupList[currentGroupNum_];

		ImGui::Dummy({0.0f,4.0f});
		if(currentGroup_){
			for(std::map<std::string,Item>::iterator itemItr = currentGroup_->begin();
				itemItr != currentGroup_->end();
				++itemItr){
				const std::string& itemName = itemItr->first;
				Item& item = itemItr->second;
				if(std::holds_alternative<int32_t>(item)){
					int32_t* valuePtr = std::get_if<int32_t>(&item);
					ImGui::DragInt(itemName.c_str(),valuePtr,1);
				} else if(std::holds_alternative<float>(item)){
					float* valuePtr = std::get_if<float>(&item);
					ImGui::DragFloat(itemName.c_str(),valuePtr,0.1f);
				} else if(std::holds_alternative<std::string>(item)){
					std::string* valuePtr = std::get_if<std::string>(&item);
					ImGui::InputText(itemName.c_str(),&valuePtr->operator[](0),sizeof(char) * 64);
				} else if(std::holds_alternative<Vec2f>(item)){
					Vec2f* valuePtr = std::get_if<Vec2f>(&item);
					ImGui::DragFloat2(itemName.c_str(),reinterpret_cast<float*>(valuePtr),0.1f);
				} else if(std::holds_alternative<Vec3f>(item)){
					Vec3f* valuePtr = std::get_if<Vec3f>(&item);
					ImGui::DragFloat3(itemName.c_str(),reinterpret_cast<float*>(valuePtr),0.1f);
				} else if(std::holds_alternative<Vec4f>(item)){
					Vec4f* valuePtr = std::get_if<Vec4f>(&item);
					ImGui::DragFloat4(itemName.c_str(),reinterpret_cast<float*>(valuePtr),0.1f);
				} else if(std::holds_alternative<bool>(item)){
					bool* valuePtr = std::get_if<bool>(&item);
					ImGui::Checkbox(itemName.c_str(),valuePtr);
				}
			}
		}

	}
	ImGui::End();
#endif // _DEBUG
}

#pragma region"Load"
void GlobalVariables::LoadAllFile(){
	std::filesystem::directory_iterator sceneDirItr(kDirectoryPath);
	for(auto& sceneEntry : sceneDirItr){
		const std::filesystem::path& scenePath = sceneEntry.path();
		std::filesystem::directory_iterator dirItr(sceneEntry);
		for(auto& entry : dirItr){
			const std::filesystem::path& filePath = entry.path();
			std::string extension = filePath.extension().string();

			if(extension.compare(".json") != 0){
				continue;
			}

			LoadFile(scenePath.stem().string(),filePath.stem().string());
		}
	}
}

void GlobalVariables::LoadFile(const std::string& scene,const std::string& groupName){
	std::string dir = kDirectoryPath + scene + "/" + groupName + ".json";
	if(!std::filesystem::exists(dir)){
		return;
	}

	std::ifstream ifs;
	ifs.open(dir);
	if(!ifs.is_open()){
		std::string message = "Failed open data file for Load.";
		MessageBoxA(nullptr,message.c_str(),"GrobalVariables",0);
		assert(0);
		return;
	}

	json root;
	ifs >> root;

	ifs.close();

	json::iterator groupItr = root.find(groupName);
	assert(groupItr != root.end());

	json::iterator itemItr = groupItr->begin();
	for(; itemItr != groupItr->end(); ++itemItr){
		std::string itemName = itemItr.key();

		if(itemItr->is_number_integer()){
			// int32_t なら
			int32_t value = itemItr->get<int32_t>();
			setValue(scene,groupName,itemName,value);
		} else if(itemItr->is_number_float()){
			// float なら
			double value = itemItr->get<double>();
			float fValue = static_cast<float>(value);
			setValue(scene,groupName,itemName,fValue);
		} else if(itemItr->is_array()){
			switch(itemItr->size()){
				case 2:
					{
						// Vec2f なら
						Vec2f value(itemItr->at(0).get<float>(),itemItr->at(1).get<float>());
						setValue(scene,groupName,itemName,value);
						break;
					}
				case 3:
					{
						// Vec3f なら
						Vec3f value(itemItr->at(0).get<float>(),itemItr->at(1).get<float>(),itemItr->at(2).get<float>());
						setValue(scene,groupName,itemName,value);
						break;
					}
				case 4:
					{
						// Vec4f なら
						Vec4f value(itemItr->at(0).get<float>(),itemItr->at(1).get<float>(),itemItr->at(2).get<float>(),itemItr->at(3).get<float>());
						setValue(scene,groupName,itemName,value);
						break;
					}
			}
		} else if(itemItr->is_boolean()){
			bool value = itemItr->get<bool>();
			setValue(scene,groupName,itemName,value);
		} else if(itemItr->is_string()){
			std::string value = itemItr->get<std::string>();
			setValue(scene,groupName,itemName,value);
		}
	}
}
#pragma endregion

#pragma region"Save"
void GlobalVariables::SaveScene(const std::string& scene){
	for(auto& group : data_[scene]){
		std::string dir = kDirectoryPath + scene + "/" + group.first + ".json";
		SaveFile(scene,group.first);
	}
}

void GlobalVariables::SaveFile(const std::string& scene,const std::string& groupName){
	///========================================
	///保存項目をまとめる
	///========================================
	std::map<std::string,Group>::iterator groupItr = data_[scene].find(groupName);
	assert(groupItr != data_[scene].end());

	json root = json::object();
	root[groupName] = json::object();

	for(std::map<std::string,Item>::iterator itemItr = groupItr->second.begin();
		itemItr != groupItr->second.end(); ++itemItr){
		const std::string& itemName = itemItr->first;
		Item& item = itemItr->second;


		if(std::holds_alternative<int32_t>(item)){
			root[groupName][itemName] = std::get<int32_t>(item);
		} else if(std::holds_alternative<float>(item)){
			root[groupName][itemName] = std::get<float>(item);
		} else if(std::holds_alternative<std::string>(item)){
			root[groupName][itemName] = std::get<std::string>(item);
		} else if(std::holds_alternative<Vec2f>(item)){
			Vec2f value = std::get<Vec2f>(item);
            root[groupName][itemName] = json::array({value.x(), value.y()});
		} else if(std::holds_alternative<Vec3f>(item)){
			Vec3f value = std::get<Vec3f>(item);
            root[groupName][itemName] = json::array({value.x(), value.y(), value.z()});
		} else if(std::holds_alternative<Vec4f>(item)){
			Vec4f value = std::get<Vec4f>(item);
            root[groupName][itemName] = json::array({value.x(), value.y(), value.z(), value.w()});
		} else if(std::holds_alternative<bool>(item)){
			root[groupName][itemName] = std::get<bool>(item);
		}

		///========================================
		/// ファイルへ書き出す
		///========================================
		std::string path = kDirectoryPath + scene + "/";
		std::filesystem::path dir(path);
		if(!std::filesystem::exists(dir)){
			std::filesystem::create_directory(dir);
		}
		std::string filePath = path + groupName + ".json";

		std::ofstream ofs;
		ofs.open(filePath);
		if(ofs.fail()){
			std::string message = "Failed open data file for write.";
			MessageBoxA(nullptr,message.c_str(),"GlobalVariables",0);
			assert(0);
			return;
		}

		ofs << std::setw(4) << root << std::endl;
		ofs.close();
	}
}
#pragma endregion

#ifdef _DEBUG
void GlobalVariables::ImGuiMenu(){
	if(ImGui::BeginMenuBar()){
		if(ImGui::BeginMenu("File")){
			if(ImGui::MenuItem("Save")){
				std::string message = std::format("{}/{}.json save it?",currentScene_,currentGroupName_);
				if(MessageBoxA(nullptr,message.c_str(),"GlobalVariables",MB_OKCANCEL) == IDOK){
					SaveFile(currentScene_,currentGroupName_);
					message = std::format("{}/{}.json saved",currentScene_,currentGroupName_);
					MessageBoxA(nullptr,message.c_str(),"GlobalVariables",MB_OK);
				}
			}
			ImGui::EndMenu();  // Ensure this is called to match BeginMenu
		}

		if(ImGui::BeginMenu("Scene")){
			if(ImGui::BeginMenu("Open")){
				std::vector<std::string> sceneList;  // Create a dynamic scene list
				for(auto& scene : data_){
					sceneList.push_back(scene.first);
				}
				for(int i = 0; i < sceneList.size(); ++i){
					if(ImGui::MenuItem(sceneList[i].c_str(),nullptr,currentSceneNum_ == i)){
						currentScene_ = sceneList[i];
						currentSceneNum_ = i; // Update currentSceneNum_ index
						currentGroupNum_ = 0;
					}
				}
				ImGui::EndMenu();  // Ensure this is called to match BeginMenu
			}
			if(ImGui::MenuItem("Save")){
				std::string message = std::format("{} save it?",currentScene_);
				if(MessageBoxA(nullptr,message.c_str(),"GlobalVariables",MB_OKCANCEL) == IDOK){
					SaveScene(currentScene_);
					message = std::format("{} saved",currentScene_);
					MessageBoxA(nullptr,message.c_str(),"GlobalVariables",MB_OK);
				}
			}
			ImGui::EndMenu();  // Ensure this is called to match BeginMenu
		}

		ImGui::EndMenuBar();  // Ensure this is called to match BeginMenuBar
	}
}
#endif // _DEBUG
