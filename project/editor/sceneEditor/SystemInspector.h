#pragma once

#ifdef _DEBUG

#include "editor/IEditor.h"

/// ECS
// system
#include "SceneEditor.h"
#include "system/ISystem.h"

/// util
#include "util/EnumBitmask.h"

/// <summary>
/// システムを表示・編集するエリア
/// </summary>
class SystemInspectorArea
    : public Editor::Area {
public:
    SystemInspectorArea(SceneEditorWindow* _window);
    ~SystemInspectorArea() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
    /// <summary>
    /// システムの実行優先度を変更するコマンド
    /// </summary>
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
    /// <summary>
    /// システムのアクティビティ状態を変更するコマンド
    /// </summary>
    class ChangeSystemActivity
        : public IEditCommand {
    public:
        ChangeSystemActivity(SystemInspectorArea* _inspectorArea, const std::string& _systemName, int32_t _systemPriority_, bool _oldActivity, bool _newActivity);
        ~ChangeSystemActivity() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        std::string systemName_; // 対象のシステム名
        int32_t systemPriority_ = 0;
        bool oldActivity_       = false; // 変更前のアクティビティ状態
        bool newActivity_       = true; // 変更後のアクティビティ状態
    };
    /// <summary>
    /// システムカテゴリのアクティビティ状態を変更するコマンド
    /// </summary>
    class ChangeSystemCategoryActivity
        : public IEditCommand {
    public:
        ChangeSystemCategoryActivity(SystemInspectorArea* _inspectorArea, OriGine::SystemCategory _category, bool _oldActivity, bool _newActivity);
        ~ChangeSystemCategoryActivity();
        void Execute();
        void Undo();

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        OriGine::SystemCategory category_; // 対象のシステムカテゴリ
        bool oldActivity_ = false; // 変更前のアクティビティ状態
        bool newActivity_ = true; // 変更後のアクティビティ状態
    };

    enum class FilterType : int32_t {
        NONE                     = 0b0, // フィルターなし
        CATEGORY_INITIALIZE      = 0b1 << 1,
        CATEGORY_INPUT           = 0b1 << 2,
        CATEGORY_STATETRANSITION = 0b1 << 3,
        CATEGORY_MOVEMENT        = 0b1 << 4,
        CATEGORY_COLLISION       = 0b1 << 5,
        CATEGORY_EFFECT          = 0b1 << 6,
        CATEGORY_RENER           = 0b1 << 7,
        CATEGORY_POSTRENDER      = 0b1 << 8,
        CATEGORY_ALL             = static_cast<int32_t>(CATEGORY_INITIALIZE) | static_cast<int32_t>(CATEGORY_INPUT) | static_cast<int32_t>(CATEGORY_STATETRANSITION) | static_cast<int32_t>(CATEGORY_MOVEMENT) | static_cast<int32_t>(CATEGORY_COLLISION) | static_cast<int32_t>(CATEGORY_EFFECT) | static_cast<int32_t>(CATEGORY_RENER) | static_cast<int32_t>(CATEGORY_POSTRENDER),
        SEARCH                   = 0b1 << 9, // 検索フィルター
    };
    /// <summary>
    /// システムを表示するフィルターを変更するコマンド
    /// </summary>
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
    /// <summary>
    /// 検索フィルターを変更するコマンド
    /// </summary>
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
    void SystemGui(const std::string& _systemName, int32_t& _priority); // システムのGUIを描画

protected:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ
    std::array<std::vector<std::pair<std::string, int32_t>>, size_t(OriGine::SystemCategory::Count)> systemMap_; // システムのマップ

    const int32_t searchBufferSize_ = 256; // 検索バッファのサイズ
    std::string searchBuffer_; // 検索用のバッファ
    EnumBitmask<FilterType> filter_ = EnumBitmask<FilterType>(FilterType::CATEGORY_ALL); // フィルターの種類
public:
    SceneEditorWindow* GetParentWindow() const {
        return parentWindow_;
    }
    const std::array<std::vector<std::pair<std::string, int32_t>>, size_t(OriGine::SystemCategory::Count)>& GetSystemMap() const {
        return systemMap_;
    }
};

#endif // _DEBUG
