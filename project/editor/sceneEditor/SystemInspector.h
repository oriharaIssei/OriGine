#pragma once

#include "editor/IEditor.h"
class SceneEditorWindow;

/// engine

// system
#include "system/ISystem.h"

/// util
#include "util/EnumBitMask.h"

class SystemInspectorArea
    : public Editor::Area {
public:
    SystemInspectorArea(SceneEditorWindow* _window);
    ~SystemInspectorArea() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
    class ChangeSystemPriority
        : public IEditCommand {
    public:
        ChangeSystemPriority(SystemInspectorArea* _inspectorArea, const std::string& _systemName, int32_t _oldPriority, int32_t _newPriority);
        ~ChangeSystemPriority() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        std::string systemName_; // 対象のシステム名
        int32_t oldPriority_ = 0; // 変更前の優先度
        int32_t newPriority_ = 0; // 変更後の優先度
    };
    class ChangeSystemActivity
        : public IEditCommand {
    public:
        ChangeSystemActivity(SystemInspectorArea* _inspectorArea, const std::string& _systemName, bool _oldActivity, bool _newActivity);
        ~ChangeSystemActivity() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        std::string systemName_; // 対象のシステム名
        bool oldActivity_ = false; // 変更前のアクティビティ状態
        bool newActivity_ = true; // 変更後のアクティビティ状態
    };

    enum class FilterType : int32_t {
        NONE                     = 0b0, // フィルターなし
        ACTIVE                   = 0b1 << 1,
        INACTIVE                 = 0b1 << 2,
        CATEGORY_INITIALIZE      = 0b1 << 3,
        CATEGORY_INPUT           = 0b1 << 4,
        CATEGORY_STATETRANSITION = 0b1 << 5,
        CATEGORY_MOVEMENT        = 0b1 << 6,
        CATEGORY_COLLISION       = 0b1 << 7,
        CATEGORY_EFFECT          = 0b1 << 8,
        CATEGORY_RENER           = 0b1 << 9,
        CATEGORY_POSTRENDER      = 0b1 << 10,
        CATEGORY_ALL             = static_cast<int32_t>(CATEGORY_INITIALIZE) | static_cast<int32_t>(CATEGORY_INPUT) | static_cast<int32_t>(CATEGORY_STATETRANSITION) | static_cast<int32_t>(CATEGORY_MOVEMENT) | static_cast<int32_t>(CATEGORY_COLLISION) | static_cast<int32_t>(CATEGORY_EFFECT) | static_cast<int32_t>(CATEGORY_RENER) | static_cast<int32_t>(CATEGORY_POSTRENDER),
        SEARCH                   = 0b1 << 11, // 検索フィルター
    };
    class ChangeSystemFilter
        : public IEditCommand {
    public:
        ChangeSystemFilter(SystemInspectorArea* _inspectorArea, int32_t _newFilter)
            : inspectorArea_(_inspectorArea), newFilter_(_newFilter) {
            oldFilter_ = static_cast<int32_t>(inspectorArea_->filter_);
        }
        ~ChangeSystemFilter() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        int32_t oldFilter_; // 変更前のフィルター
        int32_t newFilter_; // 変更後のフィルター
    };
    class ChangeSearchFilter
        : public IEditCommand {
    public:
        ChangeSearchFilter(SystemInspectorArea* _inspectorArea, const std::string& _oldBuffer)
            : inspectorArea_(_inspectorArea), oldSearchBuffer_(_oldBuffer) {
            newSearchBuffer_ = inspectorArea_->searchBuffer_;
        }
        ~ChangeSearchFilter() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        std::string oldSearchBuffer_; // 変更前の検索バッファ
        std::string newSearchBuffer_; // 変更後の検索バッファ
    };

protected:
    void SystemGui(const std::string& _systemName, ISystem* _system); // システムのGUIを描画

protected:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ
    std::array<std::unordered_map<std::string, ISystem*>, size_t(SystemCategory::Count)> systemMap_; // システムのマップ

    const int32_t searchBufferSize_ = 256; // 検索バッファのサイズ
    std::string searchBuffer_; // 検索用のバッファ
    EnumBitmask<FilterType> filter_ = EnumBitmask<FilterType>(FilterType::CATEGORY_ALL); // フィルターの種類
public:
    SceneEditorWindow* getParentWindow() const {
        return parentWindow_;
    }
};
