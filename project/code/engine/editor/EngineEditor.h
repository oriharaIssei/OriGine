#pragma once

#ifdef _DEBUG

///stl
#include <memory>
#include <string>
#include <unordered_map>

///engine
#include "module/editor/IEditor.h"
class Camera;
class Engine;
class IScene;

///=============================================================================
/// EngineEditor (Engine の 汎用的な editor郡)
///=============================================================================
class EngineEditor {
public:
    static EngineEditor* getInstance() {
        static EngineEditor instance;
        return &instance;
    }

    void Update();

private:
    EngineEditor();
    EngineEditor(const EngineEditor&) = delete;
    ~EngineEditor();
    EngineEditor& operator=(const EngineEditor&) = delete;
    EngineEditor& operator=(EngineEditor&&)      = delete;

private:
    bool isActive_ = true;

    std::unordered_map<std::string, std::unique_ptr<IEditor>> editors_;
    std::unordered_map<IEditor*, bool> editorActive_;

public:
    void SetActive(bool active) { isActive_ = active; }
    bool IsActive() const { return isActive_; }

    void addEditor(const std::string& name, std::unique_ptr<IEditor>&& editor);
};

#endif // _DEBUG
