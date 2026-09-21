#include "component/FieldStrategy.h"

/// stl
#include <array>
#include <memory>
#include <string>

/// logger
#include "logger/Logger.h"

#ifdef ORIGINE_EDITOR_ENABLED
#include <format>

/// エディタ専用のImGuiラッパー(Undo/Redo付きコマンド一式はここ経由でeditor/IEditor.hも入る)。
/// Transform.cpp 等、editor/ 以外の.cppからも同じ形でeditor専用ヘッダを取り込む前例がある
/// (ORIGINE_EDITOR_ENABLEDがDebug/Develop/Releaseの全プロジェクトで揃っているため安全)。
#include "myGui/MyGui.h"

/// 入れ子構造体(NestedStructTag)のEditは「もう1段中身を描く」だけなので、既存の汎用ドロワー
/// (DrawComponentFieldsViaDescriptor)をそのまま再帰的に呼ぶ。ECS/component から
/// editor/sceneEditor のヘッダへ依存する向きは通常と逆(層としては editor 側が ECS に依存する
/// のが自然)だが、ここは「ループを書き直さず再利用する」ことを優先した割り切りで、
/// 呼び出しは常に相互再帰(外側→ここ→DrawComponentFieldsViaDescriptor→(入れ子があれば)ここ…)
/// になる。ORIGINE_EDITOR_ENABLED で両者とも丸ごと消えるため、非エディタ構成には影響しない。
#include "editor/sceneEditor/ComponentDescriptorDrawer.h"
#endif

namespace OriGine {

namespace {

// ============================================================================
// 共通ヘルパー
// ============================================================================

/// <summary>
/// nlohmann の ADL(to_json/from_json)またはネイティブ対応(算術型・std::string・
/// std::vector<T>)にそのまま任せる素朴な往復実装。Vec2f/Vec3f/Vec4f/Quaternion は
/// math/Vector.h の to_json/from_json(Vector<N,T>基底への暗黙変換込み)がADLで見つかる。
/// Matrix3x3/Matrix4x4のようにto_jsonが無い型はこれを使わず、専用の特殊化で
/// 「保存非対応」を明示する。
/// </summary>
template <typename T>
struct JsonRoundTrip {
    static void Save(nlohmann::json& _outJson, const FieldDesc& _field, const void* _fieldPtr) {
        _outJson[_field.jsonKey_] = *static_cast<const T*>(_fieldPtr);
    }
    static void Load(const nlohmann::json& _value, void* _fieldPtr) {
        _value.get_to(*static_cast<T*>(_fieldPtr));
    }
};

/// <summary>表経由で保存できない型タグなのに保存対象になっている場合の安全網(Save側)。</summary>
void LogUnsupportedSave(const FieldDesc& _field) {
    LOG_ERROR("ToJsonViaDescriptor: フィールド '{}' は表経由で保存できない型タグ({})なのに保存対象になっている",
        _field.name_, _field.typeTag_);
}

/// <summary>LogUnsupportedSave の読み込み版。</summary>
void LogUnsupportedLoad(const FieldDesc& _field) {
    LOG_ERROR("FromJsonViaDescriptor: フィールド '{}' は表経由で読み込めない型タグ({})なのに保存対象になっている",
        _field.name_, _field.typeTag_);
}

#ifdef ORIGINE_EDITOR_ENABLED
// 「編集できない/読み取り専用」の注記に使う色。ComponentDescriptorDrawer.cpp 側にも
// 同じ定数があるが、別ファイル(別TU)のローカル定数なので値を合わせているだけの意図的な重複。
const ImVec4 kNoteColor(0.85f, 0.75f, 0.3f, 1.0f);

/// <summary>フィールド名を添えて、編集できない理由を1行で出す(黙って何も出さないための共通経路)。</summary>
void DrawFieldNote(const FieldDesc& _field, const char* _reason) {
    ::ImGui::TextColored(kNoteColor, "%s : %s", _field.name_, _reason);
}

/// <summary>
/// FieldDesc::size_ が期待する C++ 型の sizeof と一致するかを見る。一致しなければ理由を表示して
/// false を返す(呼び出し側はこの戻り値が false のとき、そのフィールドへは一切書き込んではならない)。
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

/// <summary>Matrix3x3/Matrix4x4 共通の読み取り専用グリッド描画。</summary>
void DrawReadOnlyMatrix(const FieldDesc& _field, const float* _m, int _rows, int _cols, int _stride) {
    ::ImGui::Text("%s (読み取り専用)", _field.name_);
    ::ImGui::Indent();
    for (int row = 0; row < _rows; ++row) {
        std::string line;
        for (int col = 0; col < _cols; ++col) {
            line += std::format("{:.3f}  ", _m[row * _stride + col]);
        }
        ::ImGui::TextUnformatted(line.c_str());
    }
    ::ImGui::Unindent();
}

/// <summary>
/// EnumAs&lt;U&gt; の U(uint8_t/16/32/64)を ::ImGui::InputScalar が要求する ImGuiDataType へ
/// 対応付ける。プライマリテンプレートは定義を持たない(EnumAs&lt;U&gt; が対応する4特殊化は
/// FieldTypeList 側と1対1なので、ここでも書き忘れはコンパイルエラーになる)。実行時分岐は
/// 発生しない(U はコンパイル時に決まるテンプレート引数なので、::value は定数として埋め込まれる)。
/// </summary>
template <typename U>
struct ImGuiDataTypeOf;
template <>
struct ImGuiDataTypeOf<uint8_t> {
    static constexpr ImGuiDataType value = ImGuiDataType_U8;
};
template <>
struct ImGuiDataTypeOf<uint16_t> {
    static constexpr ImGuiDataType value = ImGuiDataType_U16;
};
template <>
struct ImGuiDataTypeOf<uint32_t> {
    static constexpr ImGuiDataType value = ImGuiDataType_U32;
};
template <>
struct ImGuiDataTypeOf<uint64_t> {
    static constexpr ImGuiDataType value = ImGuiDataType_U64;
};
#endif

} // namespace

// ============================================================================
// FieldTypeList の各具体型に対する特殊化
// ============================================================================

/// <summary>bool: CheckBoxCommand(既存のUndo/Redo対応ラッパー)。</summary>
template <>
class FieldStrategy<bool> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<bool>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<bool>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<bool>(f)) return false;
        CheckBoxCommand(label, *static_cast<bool*>(p));
        return true;
    }
#endif
};

/// <summary>int32_t: DragGuiCommand&lt;int&gt;。</summary>
template <>
class FieldStrategy<int32_t> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<int32_t>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<int32_t>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<int32_t>(f)) return false;
        DragGuiCommand<int>(label, *reinterpret_cast<int32_t*>(p));
        return true;
    }
    /// <summary>std::vector&lt;int32_t&gt; の要素描画から再利用する、コマンド無しの生ウィジェット。</summary>
    static void DrawRaw(const std::string& label, int32_t& value) { DragGui<int>(label, value); }
#endif
};

/// <summary>uint32_t: DragGuiCommand&lt;unsigned int&gt;。</summary>
template <>
class FieldStrategy<uint32_t> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<uint32_t>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<uint32_t>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<uint32_t>(f)) return false;
        DragGuiCommand<unsigned int>(label, *reinterpret_cast<uint32_t*>(p));
        return true;
    }
#endif
};

/// <summary>
/// uint64_t: DragGui&lt;T&gt;はunsigned intまでしか対応しておらず、Undo/Redo付きの
/// コマンドラッパーが無い。Enumの8バイト分岐と同じ ::ImGui::InputScalar を直接使う
/// (Undo/Redoが効かない点はstd::stringと同じ既知の割り切り。textHashのような
/// no_saveフィールド用途がほとんどで、実害は小さいと判断した)。
/// </summary>
template <>
class FieldStrategy<uint64_t> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<uint64_t>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<uint64_t>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<uint64_t>(f)) return false;
        ::ImGui::InputScalar(label.c_str(), ImGuiDataType_U64, p);
        return true;
    }
#endif
};

/// <summary>float: DragGuiCommand&lt;float&gt;。</summary>
template <>
class FieldStrategy<float> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<float>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<float>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<float>(f)) return false;
        DragGuiCommand<float>(label, *reinterpret_cast<float*>(p));
        return true;
    }
    static void DrawRaw(const std::string& label, float& value) { DragGui<float>(label, value); }
#endif
};

/// <summary>Vec2f/Vec3f/Vec4f: DragGuiVectorCommand&lt;N,float&gt;。</summary>
template <>
class FieldStrategy<Vec2f> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<Vec2f>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<Vec2f>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<Vec2f>(f)) return false;
        // Vec2f(=Vector2<float>)はVector<2,float>を単一継承しているだけの派生型なので、
        // Vec2f&からVector<2,float>&への束縛は通常の基底クラス参照束縛(未定義動作なし)。
        DragGuiVectorCommand<2, float>(label, *reinterpret_cast<Vec2f*>(p));
        return true;
    }
    static void DrawRaw(const std::string& label, Vec2f& value) { DragVectorGui<2, float>(label, value); }
#endif
};

template <>
class FieldStrategy<Vec3f> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<Vec3f>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<Vec3f>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<Vec3f>(f)) return false;
        DragGuiVectorCommand<3, float>(label, *reinterpret_cast<Vec3f*>(p));
        return true;
    }
    static void DrawRaw(const std::string& label, Vec3f& value) { DragVectorGui<3, float>(label, value); }
#endif
};

template <>
class FieldStrategy<Vec4f> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<Vec4f>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<Vec4f>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<Vec4f>(f)) return false;
        DragGuiVectorCommand<4, float>(label, *reinterpret_cast<Vec4f*>(p));
        return true;
    }
    static void DrawRaw(const std::string& label, Vec4f& value) { DragVectorGui<4, float>(label, value); }
#endif
};

/// <summary>
/// Quaternion: Vec4fと違い、編集を確定したところで単位長へ戻す(afterFunc)。単位長でない
/// クォータニオンを行列にすると意図しない拡大縮小が混ざるため。Transform::UpdateMatrix() が
/// 同じ補正を持っているが、それを毎フレーム呼ぶ経路がエディタ側に無く、編集した値がそのまま
/// 残って壊れる(2026-09-21にユーザーが踏んだ)。afterFuncはSetterCommandから呼ばれるので
/// Undo/Redoでも同じ補正がかかる。
/// </summary>
template <>
class FieldStrategy<Quaternion> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<Quaternion>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<Quaternion>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<Quaternion>(f)) return false;
        DragGuiVectorCommand<4, float>(
            label,
            *reinterpret_cast<Quaternion*>(p),
            0.01f, 0.0f, 0.0f, "%.3f",
            [](Vector<4, float>* _value) {
                // 全成分を0までドラッグしても、Quaternion::Normalizeが長さ0のとき
                // 単位クォータニオンを返すのでNaNにはならない。
                Quaternion* q = static_cast<Quaternion*>(_value);
                *q            = Quaternion::Normalize(*q);
            });
        return true;
    }
#endif
};

/// <summary>
/// Matrix3x3/Matrix4x4: to_json/from_jsonが存在しない(プロジェクト内を検索して確認済み)ため、
/// 保存は明示的に非対応として扱う。エディタ表示は常に読み取り専用の数値グリッドだが、
/// Edit()の戻り値は「編集ウィジェットを描けたか」という契約どおり false を返す(呼び出し側の
/// ループはこれを見て「保存対象外につき読み取り専用」の注記を出さない。呼び出し側が
/// タグを見て特別扱いする分岐を持たずに済むのはこのため。no_saveのときに
/// DisabledScopeで薄く表示されるようになるのは許容する)。
/// </summary>
template <>
class FieldStrategy<Matrix3x3> final : public IFieldStrategy {
public:
    void Save(nlohmann::json&, const FieldDesc& f, const void*) const override { LogUnsupportedSave(f); }
    void Load(const nlohmann::json&, const FieldDesc& f, void*) const override { LogUnsupportedLoad(f); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string&, void* p) const override {
        if (!CheckFieldSize<Matrix3x3>(f)) return false;
        const Matrix3x3& m = *static_cast<const Matrix3x3*>(p);
        DrawReadOnlyMatrix(f, &m.m[0][0], 3, 3, 3);
        return false; // 編集ウィジェットは描いていない(常に読み取り専用のため)
    }
#endif
};

template <>
class FieldStrategy<Matrix4x4> final : public IFieldStrategy {
public:
    void Save(nlohmann::json&, const FieldDesc& f, const void*) const override { LogUnsupportedSave(f); }
    void Load(const nlohmann::json&, const FieldDesc& f, void*) const override { LogUnsupportedLoad(f); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string&, void* p) const override {
        if (!CheckFieldSize<Matrix4x4>(f)) return false;
        const Matrix4x4& m = *static_cast<const Matrix4x4*>(p);
        DrawReadOnlyMatrix(f, &m.m[0][0], 4, 4, 4);
        return false; // 編集ウィジェットは描いていない(常に読み取り専用のため)
    }
#endif
};

/// <summary>std::string: Undo/Redo対応コマンドが無いため直接書き込む(既知の割り切り)。</summary>
template <>
class FieldStrategy<std::string> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override { JsonRoundTrip<std::string>::Save(j, f, p); }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override { JsonRoundTrip<std::string>::Load(v, p); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<std::string>(f)) return false;
        ::ImGui::InputText(label.c_str(), static_cast<std::string*>(p));
        return true;
    }
#endif
};

/// <summary>
/// std::vector&lt;T&gt;(T = float/int32_t/Vec2f/Vec3f/Vec4f): Save/LoadはnlohmannのSTLコンテナ
/// ネイティブ対応(要素のto_json/from_jsonはADLで見つかる)に任せる。Editは要素ごとに
/// FieldStrategy&lt;T&gt;::DrawRawを呼んで再描画コードを書かない。
///
/// Undo/Redoの安全性: SetterCommand&lt;T&gt;(editor/IEditor.h)は編集対象を生ポインタT*で
/// 保持する。要素ごとにコマンドを作ると&vec[i]を持つことになり、追加/削除でvectorの内部
/// バッファが再確保されたときにUndo履歴が解放済みメモリを指す。これを避けるため、
/// 要素の編集・追加・削除はすべて「vector全体」を対象にしたSetterCommand&lt;std::vector&lt;T&gt;&gt;
/// にする(&vec自体はコンポーネント内に確保されており、vectorの中身がどう再確保されても
/// 動かない。変更前後のvectorはコピーして値として持つ)。
/// </summary>
template <typename T>
class FieldStrategy<std::vector<T>> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override {
        j[f.jsonKey_] = *static_cast<const std::vector<T>*>(p);
    }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override {
        v.get_to(*static_cast<std::vector<T>*>(p));
    }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<std::vector<T>>(f)) return false;
        auto& vec = *static_cast<std::vector<T>*>(p);

        // ボタン操作(追加/削除)は1フレームで完結するので、フレーム開始時点の値を
        // そのまま「操作前」のスナップショットとして使える。
        const std::vector<T> beforeButtonOp = vec;

        ::ImGui::Text("%s (要素数 %zu)", f.name_, vec.size());
        ::ImGui::Indent();

        bool anyActive  = false;
        int eraseIndex  = -1;
        for (size_t i = 0; i < vec.size(); ++i) {
            // label は既に "フィールド名##idSuffix_フィールド名" の形($##$以降はImGuiが表示しない
            // 隠しID)なので、末尾に足すだけだと添字が全部隠れて全行がフィールド名だけに見える。
            // 表示させたい "[i]" を先頭(##より前)に置き、一意性を保つ元のlabelは隠しID側へ回す。
            std::string elemLabel = "[" + std::to_string(i) + "]##" + label + "_" + std::to_string(i);
            // 要素単体はコマンド無しの生ウィジェットで直接書き換える(理由はクラスコメント参照)。
            FieldStrategy<T>::DrawRaw(elemLabel, vec[i]);
            if (::ImGui::IsItemActive()) {
                anyActive = true;
            }
            ::ImGui::SameLine();
            if (::ImGui::Button(("x##" + elemLabel).c_str())) {
                eraseIndex = static_cast<int>(i);
            }
        }
        bool added = ::ImGui::Button(("+ 追加##" + label).c_str());
        ::ImGui::Unindent();

        if (eraseIndex >= 0) {
            vec.erase(vec.begin() + eraseIndex);
            OriGine::EditorController::GetInstance()->PushCommand(
                std::make_unique<SetterCommand<std::vector<T>>>(&vec, vec, beforeButtonOp));
            return true;
        }
        if (added) {
            vec.push_back(T{});
            OriGine::EditorController::GetInstance()->PushCommand(
                std::make_unique<SetterCommand<std::vector<T>>>(&vec, vec, beforeButtonOp));
            return true;
        }

        // 複数フレームにまたがる要素のドラッグ編集は、DragGuiCommand等と同じ
        // GuiValuePool<T>をvector<T>に対して素直に流用する(値をキーで一時保存し、
        // 非アクティブ化を検知した時点で差分を1コマンドにする、という仕組み自体は
        // 要素数に依存しないため)。
        static GuiValuePool<std::vector<T>> valuePool;
        if (anyActive) {
            valuePool.SetValue(label, beforeButtonOp); // 既にキーがあれば何もしない(ドラッグ開始時の値を保持)
        } else if (valuePool.hasValue(label)) {
            std::vector<T> before = valuePool.popValue(label);
            OriGine::EditorController::GetInstance()->PushCommand(
                std::make_unique<SetterCommand<std::vector<T>>>(&vec, vec, before));
        }

        return true;
    }
#endif
};

/// <summary>
/// EnumAs&lt;U&gt;(U = uint8_t/16/32/64): enum を「下地と同じ幅の符号無し整数」として読み書きする
/// 印に対する唯一の部分特殊化。U はテンプレート引数として静的に決まるため、実行時の幅分岐は
/// 存在しない(旧 ReadEnumAsUInt/WriteEnumFromUInt の switch(_size) はここでは書かない。
/// EnumAs<uint8_t>/<uint16_t>/<uint32_t>/<uint64_t> の4つがそれぞれ別の
/// FieldStrategy<EnumAs<U>> インスタンスとしてテーブルに並ぶため、呼び出し側は
/// GetFieldStrategy(tag) で既に幅ごとの実体を引いている)。
///
/// Save は旧コードと同じく常に uint64_t として JSON に書く(JSON表現は変えていない)。
/// 列挙値の名前一覧はディスクリプタにまだ無い(docs/plans/phase-03c-design.md 決定3)ため、
/// Edit は下地の整数をそのまま編集する(名前付きのComboにするのは値名の表が用意されてから)。
/// </summary>
template <typename U>
class FieldStrategy<EnumAs<U>> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override {
        j[f.jsonKey_] = static_cast<uint64_t>(*static_cast<const U*>(p));
    }
    void Load(const nlohmann::json& v, const FieldDesc&, void* p) const override {
        uint64_t raw = 0;
        v.get_to(raw);
        *static_cast<U*>(p) = static_cast<U>(raw);
    }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        if (!CheckFieldSize<U>(f)) return false;
        ::ImGui::InputScalar(label.c_str(), ImGuiDataTypeOf<U>::value, p);
        ::ImGui::SameLine();
        ::ImGui::TextDisabled("(enum: 値名未対応、生の整数として編集)");
        return true;
    }
#endif
};

// ============================================================================
// NestedStructTag(入れ子の注釈付き構造体、ORIGINE_STRUCT() を指すフィールド用)
// ============================================================================

namespace {

/// <summary>
/// _field.nestedTypeIndex_ から入れ子の TypeDesc を引く。添字が範囲外/未登録、または
/// フィールド自身の size_ と入れ子型の typeSize_ が食い違っていれば理由をログに出して
/// nullptr を返す(黙って書き込まない安全網。他の型の CheckFieldSize と同じ役割だが、
/// Save/Load はエディタ専用ではない(JSON往復は非エディタ構成でも起きる)ため
/// ORIGINE_EDITOR_ENABLED の外に置く)。
/// </summary>
const TypeDesc* ResolveNestedTypeDesc(const FieldDesc& _field) {
    const TypeDesc* table = GetNestedTypeTable();
    uint32_t count         = GetNestedTypeTableCount();
    if (!table || _field.nestedTypeIndex_ >= count) {
        LOG_ERROR("入れ子フィールド '{}' の nestedTypeIndex_({}) が範囲外(登録数{})です。",
            _field.name_, _field.nestedTypeIndex_, count);
        return nullptr;
    }
    const TypeDesc& nested = table[_field.nestedTypeIndex_];
    if (nested.typeSize_ != _field.size_) {
        LOG_ERROR("入れ子フィールド '{}' はサイズ不一致(フィールド{}バイト, 入れ子型'{}'{}バイト)のため扱いません。",
            _field.name_, _field.size_, nested.typeName_, nested.typeSize_);
        return nullptr;
    }
    return &nested;
}

} // namespace

/// <summary>
/// 入れ子の注釈付き構造体(ORIGINE_STRUCT())用。フィールドの実際のC++型が何であるかは問わない
/// (どの型でも常にこの1つのタグ・この1つのストラテジーで表す)。どの入れ子 TypeDesc を指すかは
/// FieldDesc::nestedTypeIndex_(GetNestedTypeTable() への添字)が持つ。Save/Load は
/// ToJsonViaDescriptor/FromJsonViaDescriptor をオブジェクト1段ネストして再利用するだけで、
/// 入れ子の中身の型ごとの分岐はここに書かない(switchを使わないという設計方針を、再帰の中でも保つ)。
/// </summary>
template <>
class FieldStrategy<NestedStructTag> final : public IFieldStrategy {
public:
    void Save(nlohmann::json& j, const FieldDesc& f, const void* p) const override {
        const TypeDesc* nested = ResolveNestedTypeDesc(f);
        if (!nested) {
            return; // 理由はResolveNestedTypeDesc内でログ済み
        }
        nlohmann::json nestedJson = nlohmann::json::object();
        ToJsonViaDescriptor(nestedJson, p, *nested);
        j[f.jsonKey_] = std::move(nestedJson);
    }

    void Load(const nlohmann::json& v, const FieldDesc& f, void* p) const override {
        if (!v.is_object()) {
            LOG_ERROR("FromJsonViaDescriptor: フィールド '{}' は入れ子オブジェクトを期待したが、"
                       "オブジェクトではない値だったため読み込みません。",
                f.name_);
            return;
        }
        const TypeDesc* nested = ResolveNestedTypeDesc(f);
        if (!nested) {
            return;
        }
        FromJsonViaDescriptor(v, p, *nested);
    }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& f, const std::string& label, void* p) const override {
        const TypeDesc* nested = ResolveNestedTypeDesc(f);
        if (!nested) {
            DrawFieldNote(f, "入れ子型の解決に失敗したため編集不可");
            return false;
        }
        // labelは呼び出し元(DrawComponentFieldsViaDescriptor)が既に
        // "フィールド名##idSuffix_フィールド名" の形で組み立てた、この呼び出し1回分の
        // 一意なIDを持つ文字列。これをそのまま次のDrawComponentFieldsViaDescriptor呼び出しの
        // idSuffixとして渡せば、入れ子側のフィールドのIDも自動的に一意になる
        // (idSuffixが積み重なっていくだけで、新しい採番の仕組みを足す必要が無い)。
        //
        // 親フィールドがno_saveでBeginDisabled中にこのEdit()が呼ばれている場合、ImGuiの
        // 無効化はスタック(カウンタ)で効くため、ここで改めてDisabledScopeを重ねなくても
        // この再帰の中で描く全ウィジェットは自動的に無効化された状態になる。
        bool open = ::ImGui::TreeNode(label.c_str());
        if (open) {
            DrawComponentFieldsViaDescriptor(p, *nested, label);
            ::ImGui::TreePop();
        }
        return true;
    }
#endif
};

// ============================================================================
// Opaque(FieldTypeListに載らない、型を1つに絞れない残り全部用の唯一のタグ)
// ============================================================================

/// <summary>
/// タグ値が kFieldTagOpaque のフィールド用。生ポインタ・ハンドル・コンテナ等、ディスクリプタが
/// 中身を表現できない型の安全網。何もできないことを常に明示する(黙って消さない)。
/// </summary>
class OpaqueFieldStrategy final : public IFieldStrategy {
public:
    void Save(nlohmann::json&, const FieldDesc& _field, const void*) const override { LogUnsupportedSave(_field); }
    void Load(const nlohmann::json&, const FieldDesc& _field, void*) const override { LogUnsupportedLoad(_field); }
#ifdef ORIGINE_EDITOR_ENABLED
    bool Edit(const FieldDesc& _field, const std::string&, void*) const override {
        DrawFieldNote(_field, "ディスクリプタが表現できない型のため編集不可(Opaque)");
        return false;
    }
#endif
};

// ============================================================================
// 表の構築(FieldTypeListから機械的に組み立てる。個々の型名をここに書かない)
// ============================================================================

namespace {

/// <summary>
/// FieldTypeList<Ts...> を畳み込み、Ts それぞれの FieldStrategy<Ts> ひとつずつ
/// (EnumAs<U>・NestedStructTag 込み)+ Opaque用の合計 sizeof...(Ts)+1 個を、タグ値=配列添字の
/// 順で積んだ表を作る。Enum/入れ子構造体専用のタグ・専用ストラテジーを別枠で持たないのは、
/// EnumAs<uint8_t/16/32/64>・NestedStructTag が FieldTypeList の通常のエントリとして
/// 畳み込まれるため、他の型と同じ経路でテーブルに入るから。
/// </summary>
template <typename... Ts>
std::array<std::unique_ptr<IFieldStrategy>, sizeof...(Ts) + 1> MakeStrategyTable(TypeList<Ts...>) {
    std::array<std::unique_ptr<IFieldStrategy>, sizeof...(Ts) + 1> table;
    size_t idx = 0;
    ((table[idx++] = std::make_unique<FieldStrategy<Ts>>()), ...);
    table[idx++] = std::make_unique<OpaqueFieldStrategy>();
    return table;
}

} // namespace

const IFieldStrategy* GetFieldStrategy(uint8_t _tag) {
    static const auto table = MakeStrategyTable(FieldTypeList{});
    if (_tag >= table.size()) {
        LOG_ERROR("GetFieldStrategy: 範囲外の型タグ({})", _tag);
        return nullptr;
    }
    return table[_tag].get();
}

} // namespace OriGine
