#include "MyGui.h"

#ifdef _DEBUG

/// engine
#include "asset/AssetSystem.h"
// asset
#include "asset/TextureAsset.h"
/// util
#include "myFileSystem/MyFileSystem.h"

/// math
#include <cmath>

using namespace OriGine;

bool ImGui::InputText(const char* _label, std::string* _str, ImGuiInputTextFlags _flags) {
    constexpr size_t minBufferSize   = 256;
    constexpr size_t extraBufferSize = 64;

    // バッファサイズを 現在のstr + 64 or 256 にする
    size_t bufSize = (std::max)(_str->size() + extraBufferSize, minBufferSize);
    std::vector<char> buf(bufSize);

    size_t copyLen = (std::min)(_str->size(), buf.size() - 1);
    std::memcpy(buf.data(), _str->c_str(), copyLen);
    buf[copyLen] = '\0';

    if (ImGui::InputText(_label, buf.data(), buf.size(), _flags)) {
        *_str = buf.data();
        return true;
    }
    return false;
}

bool CheckBoxCommand(const std::string& _label, bool& _value) {
    bool preValue = _value;
    if (ImGui::Checkbox(_label.c_str(), &preValue)) {
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<bool>>(&_value, preValue));
        return true;
    } else {
        return false;
    }
}

bool ButtonCommand(const std::string& _label, bool& _value) {
    if (ImGui::Button(_label.c_str())) {
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<bool>>(&_value, !_value));
        return true;
    } else {
        return false;
    }
}
Vec2f ConvertMouseToSceneView(const Vec2f& _mousePos, const ImVec2& _sceneViewPos, const ImVec2& _sceneViewSize, const Vec2f& _originalResolution) {
    // SceneView 内での相対的なマウス座標を計算
    float relativeX = _mousePos[X] - _sceneViewPos.x;
    float relativeY = _mousePos[Y] - _sceneViewPos.y;

    // SceneView のスケールを計算
    float scaleX = _originalResolution[X] / _sceneViewSize.x;
    float scaleY = _originalResolution[Y] / _sceneViewSize.y;

    // ゲーム内の座標に変換
    Vec2f gamePos;
    gamePos[X] = relativeX * scaleX;
    gamePos[Y] = relativeY * scaleY;

    return gamePos;
}

bool AskLoadTextureButton(size_t _texIndex, const std::string& _parentLabel) {
    bool ask          = false;
    std::string label = "Load Texture##" + _parentLabel;
    ask               = ImGui::Button(label.c_str());

    ask |= ImGui::ImageButton(
        ImTextureID(AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(_texIndex).srv.GetGpuHandle().ptr),
        ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
    return ask;
}
bool OpenFileDialog(const std::string& _baseDirectory, std::string& _outputDirectory, std::string& _outputFileName, const std::vector<std::string>& _extension, bool _withoutExtension, bool _withoutBaseDirectory) {
    std::string directory, fileName;
    if (myfs::SelectFileDialog(_baseDirectory, directory, fileName, _extension, _withoutExtension)) {
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
    if (myfs::SelectFileDialog(_baseDirectory, directory, fileName, _extension, _withoutExtension)) {
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
