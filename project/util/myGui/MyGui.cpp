#include "MyGui.h"

#ifdef _DEBUG

#include "myFileSystem/MyFileSystem.h"
#include "texture/TextureManager.h"

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags) {
    // バッファサイズを 現在のstr + 64 or 256 にする
    size_t bufSize = std::max<size_t>(str->size() + 64, 256);
    std::vector<char> buf(bufSize);

    size_t copyLen = std::min(str->size(), buf.size() - 1);
    std::memcpy(buf.data(), str->c_str(), copyLen);
    buf[copyLen] = '\0';

    if (ImGui::InputText(label, buf.data(), buf.size(), flags)) {
        *str = buf.data();
        return true;
    }
    return false;
}

bool CheckBoxCommand(const std::string& label, bool& value) {
    bool preValue = value;
    if (ImGui::Checkbox(label.c_str(), &preValue)) {
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<bool>>(&value, preValue));
        return true;
    } else {
        return false;
    }
}

bool ButtonCommand(const std::string& label, bool& value) {
    if (ImGui::Button(label.c_str())) {
        EditorController::getInstance()->pushCommand(std::make_unique<SetterCommand<bool>>(&value, !value));
        return true;
    } else {
        return false;
    }
}
Vec2f ConvertMouseToSceneView(const Vec2f& mousePos, const ImVec2& sceneViewPos, const ImVec2& sceneViewSize, const Vec2f& originalResolution) {
    // SceneView 内での相対的なマウス座標を計算
    float relativeX = mousePos[X] - sceneViewPos.x;
    float relativeY = mousePos[Y] - sceneViewPos.y;

    // SceneView のスケールを計算
    float scaleX = originalResolution[X] / sceneViewSize.x;
    float scaleY = originalResolution[Y] / sceneViewSize.y;

    // ゲーム内の座標に変換
    Vec2f gamePos;
    gamePos[X] = relativeX * scaleX;
    gamePos[Y] = relativeY * scaleY;

    return gamePos;
}

bool AskLoadTextureButton(int32_t _texIndex, const std::string& _parentLabel) {
    bool ask          = false;
    std::string label = "Load Texture##" + _parentLabel;
    _texIndex         = (std::max)(_texIndex, 0);
    ask               = ImGui::Button(label.c_str());

    ask |= ImGui::ImageButton(
        ImTextureID(TextureManager::getDescriptorGpuHandle(_texIndex).ptr),
        ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
    return ask;
}
bool OpenFileDialog(const std::string& _baseDirectory, std::string& _outputDirectory, std::string& _outputFileName, const std::vector<std::string>& _extension, bool _withoutExtension, bool _withoutBaseDirectory) {
    std::string directory, fileName;
    if (myfs::selectFileDialog(_baseDirectory, directory, fileName, _extension, _withoutExtension)) {
        if (_withoutBaseDirectory) {
            _outputDirectory = directory;
        } else {
            _outputFileName = _baseDirectory + "/" + directory;
        }

        _outputFileName = fileName;

        return true;
    }
    return false;
}

bool OpenFileDialog(const std::string& _baseDirectory, std::string& _outputPath, const std::vector<std::string>& _extension, bool _withoutExtension, bool _withoutBaseDirectory) {
    std::string directory, fileName;
    if (myfs::selectFileDialog(_baseDirectory, directory, fileName, _extension, _withoutExtension)) {
        if (_withoutBaseDirectory) {
            _outputPath = directory + "/" + fileName;
        } else {
            _outputPath = _baseDirectory + "/" + directory + "/" + fileName;
        }

        return true;
    }
    return false;
}

#endif // _DEBUG
