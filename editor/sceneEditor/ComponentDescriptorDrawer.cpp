#include "ComponentDescriptorDrawer.h"

#ifdef ORIGINE_EDITOR_ENABLED

/// stl
#include <cstddef>
#include <cstdint>

/// ECS: FieldTypeTag switch を置き換えたストラテジー表(D3→ストラテジーパターン化)。
#include "component/FieldStrategy.h"
#include "component/ComponentReflection.h"

/// logger
#include "logger/Logger.h"

/// externals
#include "myGui/MyGui.h"

namespace OriGine {

namespace {

// 「編集できない/読み取り専用」の注記に使う色。プロジェクト共通の警告色定数は無いため、
// ImGuiでよく使われる淡い黄色をここだけのローカル定数として置く
// (component/FieldStrategy.cpp 側にも同じ値のローカル定数がある。別TUなので意図的な重複)。
const ImVec4 kNoteColor(0.85f, 0.75f, 0.3f, 1.0f);

/// <summary>
/// ImGuiのスコープでウィジェットを丸ごと無効化するためのRAIIガード。
/// ストラテジー越しの呼び出しがどこでreturnしても確実にEndDisabled()が呼ばれるようにする
/// (手でBeginDisabled/EndDisabledを対にすると対応漏れの危険がある)。
/// </summary>
class DisabledScope {
public:
    explicit DisabledScope(bool _disabled)
        : disabled_(_disabled) {
        if (disabled_) {
            ::ImGui::BeginDisabled();
        }
    }
    ~DisabledScope() {
        if (disabled_) {
            ::ImGui::EndDisabled();
        }
    }
    DisabledScope(const DisabledScope&)            = delete;
    DisabledScope& operator=(const DisabledScope&) = delete;

private:
    bool disabled_;
};

} // namespace

void DrawComponentFieldsViaDescriptor(void* _obj, const TypeDesc& _desc, const std::string& _idSuffix) {
    const FieldDesc* fields = GetFieldTable();
    if (!fields) {
        ::ImGui::TextColored(kNoteColor, "フィールド表が未登録です(RegisterGeneratedComponentDescriptors 未呼び出し)");
        return;
    }

    std::byte* base = reinterpret_cast<std::byte*>(_obj);

    for (uint32_t i = 0; i < _desc.fieldCount_; ++i) {
        const FieldDesc& f = fields[_desc.fieldStart_ + i];
        void* fieldPtr           = base + f.offset_;
        const std::string label = std::string(f.name_) + "##" + _idSuffix + "_" + f.name_;
        // B: 灰色にする/しないは「保存するか」(kFieldFlagNoSave)ではなく「編集して良いか」
        // (kFieldFlagReadOnly)で決める。両者は独立したビットなので、no_saveだが編集可能な
        // フィールド(例: OutlineComponent::paramData)がここで初めて素通しできるようになる。
        const bool readOnly = (f.flags_ & kFieldFlagReadOnly) != 0;

        const IFieldStrategy* strategy = GetFieldStrategy(f.typeTag_);
        if (!strategy) {
            // FieldTypeList/OpaqueFieldStrategyでカバーしきれない添字
            // (=生成物が壊れている)の安全網。GetFieldStrategy側でログ済み。
            ::ImGui::TextColored(kNoteColor, "%s : 未知の型タグのため編集不可", f.name_);
            continue;
        }

        // ここに型ごとの分岐は置かない。Matrix3x3/Matrix4x4(常に読み取り専用)やOpaque
        // (理由表示のみ)のような「編集ウィジェットを描かない」型は、Edit()の戻り値契約
        // (描けたかどうか)に従って自分でfalseを返す(component/FieldStrategy.cpp)。
        // 呼び出し側はその戻り値だけを見るので、読み取り専用の型を1つ増やしてもここは変わらない。
        // read_onlyのときにMatrix/OpaqueもDisabledScopeで薄く表示されるようになるのは許容する
        // (以前は特別扱いで素通ししていたが、その分岐自体が「型を知っている」状態だった)。
        bool drewEditableWidget = false;
        {
            DisabledScope disabledGuard(readOnly);
            drewEditableWidget = strategy->Edit(f, label, fieldPtr);
        }

        if (readOnly && drewEditableWidget) {
            ::ImGui::SameLine();
            ::ImGui::TextDisabled("(読み取り専用)");
        }
    }
}

} // namespace OriGine

#endif // ORIGINE_EDITOR_ENABLED
