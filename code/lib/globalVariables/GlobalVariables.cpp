#include "GlobalVariables.h"

#include <vector>

#include <filesystem>
#include <fstream>

#include <Windows.h>

#ifdef _DEBUG
#include "imgui.h"
#endif // _DEBUG

#include <json.hpp>

using json = nlohmann::json;
const std::string kDirectoryPath = "./resource/GlobalVariables/";

GlobalVariables *GlobalVariables::getInstance(){
	static GlobalVariables instance;
	return &instance;
}

void GlobalVariables::Update(){
#ifdef _DEBUG
	if(ImGui::Begin("Global Variables",nullptr,ImGuiWindowFlags_MenuBar)){

		ImGuiMenu();

		if(data_[currentScene_].empty()){
			ImGui::End();
			return;
		}

		std::vector<const char *> groupList;  // 動的なグループリストを
		for(auto &group : data_[currentScene_]){
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
				const std::string &itemName = itemItr->first;
				Item &item = itemItr->second;
				if(std::holds_alternative<int32_t>(item.value)){
					int32_t *valuePtr = std::get_if<int32_t>(&item.value);
					ImGui::DragInt(itemName.c_str(),valuePtr,1);
					auto intPtr = std::get_if<int32_t *>(&item.valuePtr);
					**intPtr = *valuePtr;
				} else if(std::holds_alternative<float>(item.value)){
					float *valuePtr = std::get_if<float>(&item.value);
					ImGui::DragFloat(itemName.c_str(),valuePtr,0.1f);
					auto floatPtr = std::get_if<float *>(&item.valuePtr);
					**floatPtr = *valuePtr;
				} else if(std::holds_alternative<Vector2>(item.value)){
					Vector2 *valuePtr = std::get_if<Vector2>(&item.value);
					ImGui::DragFloat2(itemName.c_str(),reinterpret_cast<float *>(valuePtr),0.1f);
					auto v2Ptr = std::get_if<Vector2 *>(&item.valuePtr);
					**v2Ptr = *valuePtr;
				} else if(std::holds_alternative<Vector3>(item.value)){
					Vector3 *valuePtr = std::get_if<Vector3>(&item.value);
					ImGui::DragFloat3(itemName.c_str(),reinterpret_cast<float *>(valuePtr),0.1f);
					auto v3Ptr = std::get_if<Vector3 *>(&item.valuePtr);
					**v3Ptr = *valuePtr;
				} else if(std::holds_alternative<Vector4>(item.value)){
					Vector4 *valuePtr = std::get_if<Vector4>(&item.value);
					ImGui::DragFloat4(itemName.c_str(),reinterpret_cast<float *>(valuePtr),0.1f);
					auto v4Ptr = std::get_if<Vector4 *>(&item.valuePtr);
					**v4Ptr = *valuePtr;
				} else if(std::holds_alternative<bool>(item.value)){
					bool *valuePtr = std::get_if<bool>(&item.value);
					ImGui::Checkbox(itemName.c_str(),valuePtr);
					auto boolPtr = std::get_if<bool *>(&item.valuePtr);
					**boolPtr = *valuePtr;
				}
			}
		}

		ImGui::Dummy({0.0f,4.0f});
		ImGui::Text("\n");
		if(ImGui::Button("save")){
			SaveFile(currentScene_,currentGroupName_);

		}
	}
	ImGui::End();
#endif // _DEBUG
}

void GlobalVariables::CreateScene(const std::string &scene){
	data_[scene];
}

void GlobalVariables::LoadAllFile(){
	std::filesystem::directory_iterator sceneDirItr(kDirectoryPath);
	for(auto &sceneEntry : sceneDirItr){
		const std::filesystem::path &scenePath = sceneEntry.path();
		std::filesystem::directory_iterator dirItr(sceneEntry);
		for(auto &entry : dirItr){
			const std::filesystem::path &filePath = entry.path();
			std::string extension = filePath.extension().string();

			if(extension.compare(".json") != 0){
				continue;
			}

			LoadFile(scenePath.stem().string(),filePath.stem().string());
		}
	}
}

void GlobalVariables::LoadFile(const std::string &scene,const std::string &groupName){
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
				case 2: {
					// Vector2 なら
					Vector2 value(itemItr->at(0).get<float>(),itemItr->at(1).get<float>());
					setValue(scene,groupName,itemName,value);
					break;
				}
				case 3: {
					// Vector3 なら
					Vector3 value(itemItr->at(0).get<float>(),itemItr->at(1).get<float>(),itemItr->at(2).get<float>());
					setValue(scene,groupName,itemName,value);
					break;
				}
				case 4: {
					// Vector4 なら
					Vector4 value(itemItr->at(0).get<float>(),itemItr->at(1).get<float>(),itemItr->at(2).get<float>(),itemItr->at(3).get<float>());
					setValue(scene,groupName,itemName,value);
					break;
				}
			}
		} else if(itemItr->is_boolean()){
			bool value = itemItr->get<bool>();
			setValue(scene,groupName,itemName,value);
		}
	}
}

void GlobalVariables::SaveScene(const std::string &scene){
	for(auto &group : data_[scene]){
		std::string dir = kDirectoryPath + scene + "/" + group.first + ".json";
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

		json::iterator groupItr = root.find(group.first);
		assert(groupItr != root.end());

		json::iterator itemItr = groupItr->begin();
		for(; itemItr != groupItr->end(); ++itemItr){
			std::string itemName = itemItr.key();

			if(itemItr->is_number_integer()){
				// int32_t なら
				int32_t value = itemItr->get<int32_t>();
				setValue(scene,group.first,itemName,value);
			} else if(itemItr->is_number_float()){
				// float なら
				double value = itemItr->get<double>();
				float fValue = static_cast<float>(value);
				setValue(scene,group.first,itemName,fValue);
			} else if(itemItr->is_array() && itemItr->size()){
				// Vector3 なら
				Vector3 value(itemItr->at(0),itemItr->at(1),itemItr->at(0));
				setValue(scene,group.first,itemName,value);
			} else if(itemItr->is_boolean()){
				bool value = itemItr->get<bool>();
				setValue(scene,group.first,itemName,value);
			}
		}
	}
}

void GlobalVariables::SaveFile(const std::string &scene,const std::string &groupName){
	///========================================
	///保存項目をまとめる
	///========================================
	std::map<std::string,Group>::iterator groupItr = data_[scene].find(groupName);
	assert(groupItr != data_[scene].end());

	json root = json::object();
	root[groupName] = json::object();

	for(std::map<std::string,Item>::iterator itemItr = groupItr->second.begin();
		itemItr != groupItr->second.end(); ++itemItr){
		const std::string &itemName = itemItr->first;
		Item &item = itemItr->second;

		if(std::holds_alternative<int32_t>(item.value)){
			root[groupName][itemName] = std::get<int32_t>(item.value);
		} else if(std::holds_alternative<float>(item.value)){
			root[groupName][itemName] = std::get<float>(item.value);
		} else if(std::holds_alternative<Vector2>(item.value)){
			Vector2 value = std::get<Vector2>(item.value);
			root[groupName][itemName] = json::array({value.x,value.y});
		} else if(std::holds_alternative<Vector3>(item.value)){
			Vector3 value = std::get<Vector3>(item.value);
			root[groupName][itemName] = json::array({value.x,value.y,value.z});
		} else if(std::holds_alternative<Vector4>(item.value)){
			Vector4 value = std::get<Vector4>(item.value);
			root[groupName][itemName] = json::array({value.x,value.y,value.z,value.w});
		} else if(std::holds_alternative<bool>(item.value)){
			root[groupName][itemName] = std::get<bool>(item.value);
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
			MessageBoxA(nullptr,message.c_str(),"GrobalVariables",0);
			assert(0);
			return;
		}

		ofs << std::setw(4) << root << std::endl;
		ofs.close();
	}
}

#ifdef _DEBUG
void GlobalVariables::ImGuiMenu(){
	if(!ImGui::BeginMenuBar()){
		ImGui::EndMenuBar();
		return;
	}
	if(ImGui::BeginMenu("File")){
		if(ImGui::MenuItem("Save")){
			std::string message = std::format("{}/{}.json save it?",currentScene_,currentGroupName_);
			if(MessageBoxA(nullptr,message.c_str(),"GlobalVariables",MB_OKCANCEL)){
				SaveFile(currentScene_,currentGroupName_);
				message = std::format("{}/{}.json saveed",currentScene_,currentGroupName_);
				MessageBoxA(nullptr,message.c_str(),"GlobalVariables",0);
			}
		}
		ImGui::EndMenu();
	} else if(ImGui::BeginMenu("Scene")){
		if(ImGui::BeginMenu("Open")){
			std::vector<std::string> sceneList;  // 動的なシーンリストを作成
			for(auto &scene : data_){
				sceneList.push_back(scene.first);
			}
			for(auto &scene : sceneList){
				if(ImGui::MenuItem(scene.c_str())){
					currentScene_ = scene;
					break;
				}
			}
			currentScene_ = sceneList[currentSceneNum_];
		} else if(ImGui::MenuItem("Save")){
			std::string message = std::format("{} save it?",currentScene_);
			if(MessageBoxA(nullptr,message.c_str(),"GlobalVariables",MB_OKCANCEL)){
				SaveScene(currentScene_);
				message = std::format("{} saveed",currentScene_);
				MessageBoxA(nullptr,message.c_str(),"GlobalVariables",0);
			}
		}
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
}

#endif // _DEBUG
