#include "ComponentDescriptorDrawer.h"

#ifdef ORIGINE_EDITOR_ENABLED

/// stl
#include <cstddef>
#include <cstdint>

/// ECS
#include "component/ComponentReflection.h"

/// math
#include "math/Matrix4x4.h"
#include "math/Quaternion.h"
#include "math/Vector.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

/// logger
#include "logger/Logger.h"

/// externals
#include "myGui/MyGui.h"

namespace OriGine {

namespace {

// 「編集できない/読み取り専用」の注記に使う色。プロジェクト共通の警告色定数は無いため、
// ImGuiでよく使われる淡い黄色をここだけのローカル定数として置く。
const ImVec4 kNoteColor(0.85f, 0.75f, 0.3f, 1.0f);

/// <summary>
/// ImGuiのスコープでウィジェットを丸ごと無効化するためのRAIIガード。
/// switch内のどの分岐からbreakしても確実にEndDisabled()が呼ばれるようにする
/// (手でBeginDisabled/EndDisabledを対にすると、分岐が増えるたびに対応漏れの危険がある)。
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

/// <summary>フィールド名を添えて、編集できない理由を1行で出す(黙って何も出さないための共通経路)。</summary>
void DrawFieldNote(const FieldDesc& _field, const char* _reason) {
    ::ImGui::TextColored(kNoteColor, "%s : %s", _field.name_, _reason);
}

/// <summary>
/// FieldDesc::size_ が期待する C++ 型の sizeof と一致するかを見る。一致しなければ理由を表示して false を返す
/// (呼び出し側はこの戻り値が false のとき、そのフィールドへは一切書き込んではならない)。
/// </summary>
template <typename Expected>
bool CheckFieldSize(const FieldDesc& _field) {
    if (_field.size_ != static_cast<uint32_t>(sizeof(Expected))) {
        LOG_ERROR("DrawComponentFieldsViaDescriptor: フィールド '{}' はサイズ不一致(期待{}, 記録{})のため書き込みません。",
            _field.name_, sizeof(Expected), _field.size_);
        DrawFieldNote(_field, "サイズ不一致を検出したため編集不可");
        return false;
    }
    return true;
}

/// <summary>
/// Matrix4x4 フィールドの描画。Transform::worldMat 等、毎フレーム再計算される値なので
/// 常に読み取り専用(no_save かどうかに関わらず)。理由は ComponentDescriptorDrawer.h のコメント参照。
/// </summary>
void DrawMatrixField(const FieldDesc& _field, const void* _fieldPtr) {
    if (!CheckFieldSize<Matrix4x4>(_field)) {
        return;
    }
    const Matrix4x4& m = *reinterpret_cast<const Matrix4x4*>(_fieldPtr);
    ::ImGui::Text("%s (読み取り専用)", _field.name_);
    ::ImGui::Indent();
    for (int row = 0; row < 4; ++row) {
        ::ImGui::Text("%.3f  %.3f  %.3f  %.3f", m.m[row][0], m.m[row][1], m.m[row][2], m.m[row][3]);
    }
    ::ImGui::Unindent();
}

/// <summary>
/// Enum フィールドの描画。列挙値の名前一覧はディスクリプタにまだ無い
/// (docs/plans/phase-03c-design.md 決定3)ため、下地の整数をそのまま編集する。
/// 名前付きの Combo にするのは、値名の表が用意されてから。
/// </summary>
/// <returns>編集ウィジェットを描けたか(falseならサイズ未対応)</returns>
bool DrawEnumField(const FieldDesc& _field, const std::string& _label, void* _fieldPtr) {
    bool ok = true;
    switch (_field.size_) {
    case 1:
        ::ImGui::InputScalar(_label.c_str(), ImGuiDataType_U8, _fieldPtr);
        break;
    case 2:
        ::ImGui::InputScalar(_label.c_str(), ImGuiDataType_U16, _fieldPtr);
        break;
    case 4:
        ::ImGui::InputScalar(_label.c_str(), ImGuiDataType_U32, _fieldPtr);
        break;
    case 8:
        ::ImGui::InputScalar(_label.c_str(), ImGuiDataType_U64, _fieldPtr);
        break;
    default:
        LOG_ERROR("DrawEnumField: フィールド '{}' は未対応の enum サイズ({}バイト)です。", _field.name_, _field.size_);
        DrawFieldNote(_field, "未対応のenumサイズのため編集不可");
        ok = false;
        break;
    }
    if (ok) {
        ::ImGui::SameLine();
        ::ImGui::TextDisabled("(enum: 値名未対応、生の整数として編集)");
    }
    return ok;
}

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
        void* fieldPtr      = base + f.offset_;
        const std::string label = std::string(f.name_) + "##" + _idSuffix + "_" + f.name_;
        const bool noSave       = (f.flags_ & kFieldFlagNoSave) != 0;
        const FieldTypeTag tag  = static_cast<FieldTypeTag>(f.typeTag_);

        // Matrix4x4 は no_save の有無に関わらず常に読み取り専用(毎フレーム再計算される値のため。
        // 現行10型の中で該当するのは Transform::worldMat 等だが、たまたま全て no_save でもある)。
        if (tag == FieldTypeTag::Matrix4x4) {
            DrawMatrixField(f, fieldPtr);
            continue;
        }
        // Opaque はディスクリプタが中身を表現できない型(生ポインタ、ハンドル、コンテナ等)。
        // フィールド自体は存在するのに何も出さないのが一番悪いため、必ず理由を出す。
        if (tag == FieldTypeTag::Opaque) {
            DrawFieldNote(f, "ディスクリプタが表現できない型のため編集不可(Opaque)");
            continue;
        }

        bool drewEditableWidget = false;
        {
            DisabledScope disabledGuard(noSave);
            switch (tag) {
            case FieldTypeTag::Bool:
                drewEditableWidget = CheckFieldSize<bool>(f);
                if (drewEditableWidget) {
                    CheckBoxCommand(label, *reinterpret_cast<bool*>(fieldPtr));
                }
                break;
            case FieldTypeTag::Int32:
                drewEditableWidget = CheckFieldSize<int32_t>(f);
                if (drewEditableWidget) {
                    DragGuiCommand<int>(label, *reinterpret_cast<int32_t*>(fieldPtr));
                }
                break;
            case FieldTypeTag::UInt32:
                drewEditableWidget = CheckFieldSize<uint32_t>(f);
                if (drewEditableWidget) {
                    DragGuiCommand<unsigned int>(label, *reinterpret_cast<uint32_t*>(fieldPtr));
                }
                break;
            case FieldTypeTag::Float:
                drewEditableWidget = CheckFieldSize<float>(f);
                if (drewEditableWidget) {
                    DragGuiCommand<float>(label, *reinterpret_cast<float*>(fieldPtr));
                }
                break;
            case FieldTypeTag::Vec2f:
                drewEditableWidget = CheckFieldSize<Vec2f>(f);
                if (drewEditableWidget) {
                    // Vec2f(=Vector2<float>) は Vector<2,float> を単一継承しているだけの派生型なので、
                    // Vec2f& から Vector<2,float>& への束縛は通常の基底クラス参照束縛(未定義動作なし)。
                    DragGuiVectorCommand<2, float>(label, *reinterpret_cast<Vec2f*>(fieldPtr));
                }
                break;
            case FieldTypeTag::Vec3f:
                drewEditableWidget = CheckFieldSize<Vec3f>(f);
                if (drewEditableWidget) {
                    DragGuiVectorCommand<3, float>(label, *reinterpret_cast<Vec3f*>(fieldPtr));
                }
                break;
            case FieldTypeTag::Vec4f:
                drewEditableWidget = CheckFieldSize<Vec4f>(f);
                if (drewEditableWidget) {
                    DragGuiVectorCommand<4, float>(label, *reinterpret_cast<Vec4f*>(fieldPtr));
                }
                break;
            case FieldTypeTag::Quaternion:
                drewEditableWidget = CheckFieldSize<Quaternion>(f);
                if (drewEditableWidget) {
                    // Quaternion は Vector<4,float> を直接継承しているのでそのまま渡せる。
                    // ドラッグ操作は正規化しないため、手書きの Transform::Edit のように毎回
                    // Normalize() する補正はここでは行わない(型を知らない汎用パスであり、
                    // 「Quaternionという名前だけ知っていて中身の意味は知らない」ことを崩したくないため)。
                    DragGuiVectorCommand<4, float>(label, *reinterpret_cast<Quaternion*>(fieldPtr));
                }
                break;
            case FieldTypeTag::String:
                drewEditableWidget = CheckFieldSize<std::string>(f);
                if (drewEditableWidget) {
                    // std::string 用の Undo/Redo 対応コマンドは用意されていないため直接書き込む
                    // (EntityInformationRegion のエンティティ名編集と同程度の素朴さ)。
                    ::ImGui::InputText(label.c_str(), reinterpret_cast<std::string*>(fieldPtr));
                }
                break;
            case FieldTypeTag::Enum:
                drewEditableWidget = DrawEnumField(f, label, fieldPtr);
                break;
            default:
                // FieldTypeTag に将来追加されて生成ツールだけが対応し、こちらの対応が漏れた場合の安全網。
                LOG_ERROR("DrawComponentFieldsViaDescriptor: フィールド '{}' は未知の型タグ({})です。", f.name_, f.typeTag_);
                DrawFieldNote(f, "未知の型タグのため編集不可");
                break;
            }
        }

        if (noSave && drewEditableWidget) {
            ::ImGui::SameLine();
            ::ImGui::TextDisabled("(保存対象外につき読み取り専用)");
        }
    }
}

} // namespace OriGine

#endif // ORIGINE_EDITOR_ENABLED
