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

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags) {
    constexpr size_t minBufferSize   = 256;
    constexpr size_t extraBufferSize = 64;

    // バッファサイズを 現在のstr + 64 or 256 にする
    size_t bufSize = (std::max)(str->size() + extraBufferSize, minBufferSize);
    std::vector<char> buf(bufSize);

    size_t copyLen = (std::min)(str->size(), buf.size() - 1);
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
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<bool>>(&value, preValue));
        return true;
    } else {
        return false;
    }
}

bool ButtonCommand(const std::string& label, bool& value) {
    if (ImGui::Button(label.c_str())) {
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<bool>>(&value, !value));
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

bool AskLoadTextureButton(size_t texIndex, const std::string& parentLabel) {
    bool ask          = false;
    std::string label = "Load Texture##" + parentLabel;
    ask               = ImGui::Button(label.c_str());

    ask |= ImGui::ImageButton(
        ImTextureID(AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(texIndex).srv.GetGpuHandle().ptr),
        ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
    return ask;
}
bool OpenFileDialog(const std::string& baseDirectory, std::string& outputDirectory, std::string& outputFileName, const std::vector<std::string>& extension, bool withoutExtension, bool withoutBaseDirectory) {
    std::string directory, fileName;
    if (myfs::SelectFileDialog(baseDirectory, directory, fileName, extension, withoutExtension)) {
        if (withoutBaseDirectory) {
            outputDirectory = directory;
        } else {
            outputFileName = baseDirectory + "/" + directory;
        }

        outputFileName = fileName;

        return true;
    }
    return false;
}

bool OpenFileDialog(const std::string& baseDirectory, std::string& outputPath, const std::vector<std::string>& extension, bool withoutExtension, bool withoutBaseDirectory) {
    std::string directory, fileName;
    if (myfs::SelectFileDialog(baseDirectory, directory, fileName, extension, withoutExtension)) {
        if (withoutBaseDirectory) {
            outputPath = directory + "/" + fileName;
        } else {
            outputPath = baseDirectory + "/" + directory + "/" + fileName;
        }

        return true;
    }
    return false;
}

#endif // _DEBUG
