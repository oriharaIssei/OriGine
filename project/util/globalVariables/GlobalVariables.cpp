#include "GlobalVariables.h"

/// stl
#include <vector>
// io
#include <filesystem>
#include <fstream>

/// api
#include <Windows.h>

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// externals
#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <nlohmann/json.hpp>

using namespace OriGine;

using json                       = nlohmann::json;
const std::string kDirectoryPath = kApplicationResourceDirectory + "/GlobalVariables/";

GlobalVariables* GlobalVariables::GetInstance() {
    static GlobalVariables instance;
    return &instance;
}

GlobalVariables::GlobalVariables() {}

GlobalVariables::~GlobalVariables() {}

#pragma region "Load"
void GlobalVariables::LoadAllFile() {
    std::filesystem::directory_iterator sceneDirItr(kDirectoryPath);
    for (auto& sceneEntry : sceneDirItr) {
        const std::filesystem::path& scenePath = sceneEntry.path();
        std::filesystem::directory_iterator dirItr(sceneEntry);
        for (auto& entry : dirItr) {
            const std::filesystem::path& filePath = entry.path();
            std::string extension                 = filePath.extension().string();

            if (extension.compare(".json") != 0) {
                continue;
            }

            LoadFile(scenePath.stem().string(), filePath.stem().string());
        }
    }
}

void GlobalVariables::LoadFile(const std::string& scene, const std::string& groupName) {
    std::string dir = kDirectoryPath + scene + "/" + groupName + ".json";
    if (!std::filesystem::exists(dir)) {
        return;
    }

    std::ifstream ifs;
    ifs.open(dir);
    if (!ifs.is_open()) {
        std::string message = "Failed open data file for Load.";
        MessageBoxA(nullptr, message.c_str(), "GrobalVariables", 0);
        assert(0);
        return;
    }

    json root;
    ifs >> root;

    ifs.close();

    json::iterator groupItr = root.find(groupName);
    assert(groupItr != root.end());

    json::iterator itemItr = groupItr->begin();
    for (; itemItr != groupItr->end(); ++itemItr) {
        std::string itemName = itemItr.key();

        if (itemItr->is_number_integer()) {
            // int32_t なら
            int32_t value = itemItr->get<int32_t>();
            SetValue(scene, groupName, itemName, value);
        } else if (itemItr->is_number_float()) {
            // float なら
            double value = itemItr->get<double>();
            float fValue = static_cast<float>(value);
            SetValue(scene, groupName, itemName, fValue);
        } else if (itemItr->is_array()) {
            switch (itemItr->size()) {
            case 2: {
                // Vec2f なら
                Vec2f value(itemItr->at(0).get<float>(), itemItr->at(1).get<float>());
                SetValue(scene, groupName, itemName, value);
                break;
            }
            case 3: {
                // Vec3f なら
                Vec3f value(itemItr->at(0).get<float>(), itemItr->at(1).get<float>(), itemItr->at(2).get<float>());
                SetValue(scene, groupName, itemName, value);
                break;
            }
            case 4: {
                // Vec4f なら
                Vec4f value(itemItr->at(0).get<float>(), itemItr->at(1).get<float>(), itemItr->at(2).get<float>(), itemItr->at(3).get<float>());
                SetValue(scene, groupName, itemName, value);
                break;
            }
            }
        } else if (itemItr->is_boolean()) {
            bool value = itemItr->get<bool>();
            SetValue(scene, groupName, itemName, value);
        } else if (itemItr->is_string()) {
            std::string value = itemItr->get<std::string>();
            SetValue(scene, groupName, itemName, value);
        }
    }
}
#pragma endregion

#pragma region "Save"
void GlobalVariables::SaveScene(const std::string& scene) {
    for (auto& group : data_[scene]) {
        std::string dir = kDirectoryPath + scene + "/" + group.first + ".json";
        SaveFile(scene, group.first);
    }
}

void GlobalVariables::SaveFile(const std::string& scene, const std::string& groupName) {
    ///========================================
    /// 保存項目をまとめる
    ///========================================
    std::map<std::string, Group>::iterator groupItr = data_[scene].find(groupName);
    assert(groupItr != data_[scene].end());

    json root       = json::object();
    root[groupName] = json::object();

    for (std::map<std::string, Item>::iterator itemItr = groupItr->second.begin();
        itemItr != groupItr->second.end(); ++itemItr) {
        const std::string& itemName = itemItr->first;
        Item& item                  = itemItr->second;

        if (std::holds_alternative<int32_t>(item)) {
            root[groupName][itemName] = std::get<int32_t>(item);
        } else if (std::holds_alternative<float>(item)) {
            root[groupName][itemName] = std::get<float>(item);
        } else if (std::holds_alternative<std::string>(item)) {
            root[groupName][itemName] = std::get<std::string>(item);
        } else if (std::holds_alternative<Vec2f>(item)) {
            Vec2f value               = std::get<Vec2f>(item);
            root[groupName][itemName] = json::array({value[X], value[Y]});
        } else if (std::holds_alternative<Vec3f>(item)) {
            Vec3f value               = std::get<Vec3f>(item);
            root[groupName][itemName] = json::array({value[X], value[Y], value[Z]});
        } else if (std::holds_alternative<Vec4f>(item)) {
            Vec4f value               = std::get<Vec4f>(item);
            root[groupName][itemName] = json::array({value[X], value[Y], value[Z], value[W]});
        } else if (std::holds_alternative<bool>(item)) {
            root[groupName][itemName] = std::get<bool>(item);
        }

        ///========================================
        /// ファイルへ書き出す
        ///========================================
        std::string path = kDirectoryPath + scene + "/";
        std::filesystem::path dir(path);
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directory(dir);
        }
        std::string filePath = path + groupName + ".json";

        std::ofstream ofs;
        ofs.open(filePath);
        if (ofs.fail()) {
            std::string message = "Failed open data file for write.";
            MessageBoxA(nullptr, message.c_str(), "GlobalVariables", 0);
            assert(0);
            return;
        }

        ofs << std::setw(4) << root << std::endl;
        ofs.close();
    }
}
#pragma endregion
