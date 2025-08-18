#include "MyGui.h"

#ifdef _DEBUG

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags) {
    char buf[256];
    strncpy(buf, str->c_str(), sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';
    if (ImGui::InputText(label, buf, sizeof(buf), flags)) {
        *str = buf;
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
#endif // _DEBUG
