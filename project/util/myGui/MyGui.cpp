#include "MyGui.h"

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags) {
    // バッファサイズを 現在のstr + 64 or 256 にする
    size_t bufSize = std::max<size_t>(str->size() + 64, 256);
    std::vector<char> buf(bufSize);
    strncpy(buf.data(), str->c_str(), buf.size());
    buf[buf.size() - 1] = '\0';

    if (ImGui::InputText(label, buf.data(), buf.size(), flags)) {
        *str = buf.data();
        return true;
    }
    return false;
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
