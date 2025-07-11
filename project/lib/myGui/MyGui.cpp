#include "MyGui.h"

#ifdef _DEBUG

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
