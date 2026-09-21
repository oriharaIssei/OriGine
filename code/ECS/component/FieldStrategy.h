#pragma once

/// stl
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

/// externals
#include <nlohmann/json.hpp>

/// math(表経由の Save/Load/Edit が扱う具体型。FieldTypeList と1対1に対応する)
#include "math/Matrix3x3.h"
#include "math/Matrix4x4.h"
#include "math/Quaternion.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

/// ECS
#include "component/ComponentReflection.h"

namespace OriGine {

/// <summary>
/// コンパイル時の型の並びを保持するためだけの空クラス(実体化されない。型のリストとして
/// 使うためだけの器)。
/// </summary>
template <typename... Ts>
struct TypeList {};

/// <summary>
/// 「enum 型 E をこの符号無し整数幅 U のまま読み書きする」という指定だけを表す印。
/// 実データは U と同じメモリ表現(下地の整数をそのままの幅で符号無し扱いする。旧
/// ReadEnumAsUInt/WriteEnumFromUInt と同じ規則)だが、U 自身(uint8_t 等)は既に
/// FieldTypeList の実データ型として登録されているため、区別するためだけにこの印で包む。
/// これにより「enum である」という性質もリスト内の1エントリとして表現でき、
/// kFieldTagEnum のような専用のタグ値・専用のストラテジークラスが不要になる。
/// </summary>
template <typename U>
struct EnumAs {};

/// <summary>
/// 「このフィールドは入れ子の注釈付き構造体(ORIGINE_STRUCT())を指している」という指定だけを
/// 表す印。EnumAs&lt;U&gt;と同じ役割で、実際のC++型(例: IConstantBuffer&lt;OutlineParamData&gt;)を
/// このリストに書けないのは、入れ子構造体が何十種類あっても同じ1つのタグ値(kFieldTagNestedStruct)
/// で表せ、個々の区別は FieldDesc::nestedTypeIndex_(GetNestedTypeTable()への添字)が担うため。
/// 生成ツール(ReflectionCodeGen)はフィールドがこのタグに該当するかを decltype では判定しない
/// (=このタグは kFieldTagOf&lt;decltype(...)&gt; の結果として出てこない)。フィールドの型トークンに
/// 登録済みの構造体名が含まれるかという、生成ツール側の文字列照合で決め、生成物には
/// `OriGine::kFieldTagNestedStruct` を直接書く(component/FieldStrategy.h 参照)。
/// </summary>
struct NestedStructTag {};

/// <summary>
/// 表経由で Save/Load/Edit できる具体型の一覧(旧 FieldTypeTag switch を置き換えるストラテジー
/// パターンの唯一の型定義場所)。このリストの並び順がそのままタグの値になるため、
/// 型とタグの対応をここ以外に書かない(FieldTypeTag のような enum との二重管理をしない)。
/// 型を1つ増やすときは、ここに型を足し、対応する FieldStrategy&lt;T&gt; の特殊化
/// (component/FieldStrategy.cpp)を書くだけでよい。生成ツール(ReflectionCodeGen)は
/// decltype でこの表からタグを引くだけなので変更不要。
///
/// EnumAs&lt;uint8_t/16/32/64&gt; の4つは、実在する具体型ではなく「下地の整数幅がこれ」という
/// enum 用の印(kFieldTagOf&lt;E&gt; が std::underlying_type_t&lt;E&gt; の符号無し版から
/// このエントリの位置を引く)。これで Enum 用の専用タグ・専用ストラテジークラスをやめ、
/// 他の具体型と同じ「リストに載っている1エントリ」として扱えるようにしてある。
/// </summary>
using FieldTypeList = TypeList<
    bool, int32_t, uint32_t, uint64_t, float,
    Vec2f, Vec3f, Vec4f, Quaternion, Matrix3x3, Matrix4x4, std::string,
    std::vector<float>, std::vector<int32_t>, std::vector<Vec2f>, std::vector<Vec3f>, std::vector<Vec4f>,
    EnumAs<uint8_t>, EnumAs<uint16_t>, EnumAs<uint32_t>, EnumAs<uint64_t>,
    NestedStructTag>;

namespace FieldTypeListDetail {

/// <summary>パック中に T と同じ型があればその位置、無ければ -1 を返す。</summary>
template <typename T, typename... Ts>
constexpr int IndexInPack() {
    int idx = 0, found = -1;
    ((std::is_same_v<T, Ts> ? (found = idx, ++idx) : ++idx), ...);
    return found;
}

template <typename T, typename List>
struct IndexOf;
template <typename T, typename... Ts>
struct IndexOf<T, TypeList<Ts...>> {
    static constexpr int value = IndexInPack<T, Ts...>();
};

template <typename List>
struct ListSize;
template <typename... Ts>
struct ListSize<TypeList<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

} // namespace FieldTypeListDetail

/// <summary>
/// FieldTypeList のどれにも(EnumAs&lt;U&gt;も含めて)当てはまらない、残り全部(生ポインタ・
/// ハンドル・IConstantBuffer 等)のタグ値(旧 FieldTypeTag::Opaque と同じ、具体型に当てはまらない
/// ものをまとめて受け止めるキャッチオール)。
/// </summary>
inline constexpr uint8_t kFieldTagOpaque = static_cast<uint8_t>(FieldTypeListDetail::ListSize<FieldTypeList>::value);

namespace FieldTypeListDetail {

/// <summary>
/// T をタグ値へ変換する実装本体。列挙型の分岐(is_enum_v<T>)は if constexpr で
/// コンパイル時に確定するため、実行時分岐は発生しない。enum は
/// EnumAs&lt;make_unsigned_t&lt;underlying_type_t&lt;E&gt;&gt;&gt; という「同じ幅の符号無し整数として
/// 読み書きする」印の位置へ丸ごと委譲する(下地が符号付き enum class の既定である int でも、
/// make_unsigned で同じ幅の符号無し整数として読む。旧 ReadEnumAsUInt/WriteEnumFromUInt と
/// 同じ規則で、JSON表現(生の整数値)は変えていない)。
/// </summary>
template <typename T>
constexpr uint8_t ComputeFieldTag() {
    constexpr int idx = IndexOf<T, FieldTypeList>::value;
    if constexpr (idx >= 0) {
        return static_cast<uint8_t>(idx);
    } else if constexpr (std::is_enum_v<T>) {
        using UnsignedUnderlying = std::make_unsigned_t<std::underlying_type_t<T>>;
        constexpr int enumIdx    = IndexOf<EnumAs<UnsignedUnderlying>, FieldTypeList>::value;
        static_assert(enumIdx >= 0,
            "この下地幅の enum に対応する EnumAs<U> が FieldTypeList に無い"
            "(1/2/4/8バイト以外の下地幅は今のところ未対応)");
        return static_cast<uint8_t>(enumIdx);
    } else {
        return static_cast<uint8_t>(ListSize<FieldTypeList>::value); // kFieldTagOpaque と同じ値
    }
}

} // namespace FieldTypeListDetail

/// <summary>
/// C++ の型 T をディスクリプタのタグ値(uint8_t)へ変換する。生成ツールは
/// `kFieldTagOf<decltype(OriGine::Type::field)>` という式をそのまま出力するだけで、
/// 実際の判定はコンパイラ(decltype + テンプレート特殊化解決)に委ねる(D3の switch 廃止)。
/// </summary>
template <typename T>
inline constexpr uint8_t kFieldTagOf = FieldTypeListDetail::ComputeFieldTag<T>();

/// <summary>
/// 入れ子の注釈付き構造体(ORIGINE_STRUCT())用のタグ値。kFieldTagOf&lt;NestedStructTag&gt;と
/// 同じ値だが、生成ツールはこの名前で直接参照する(NestedStructTagという印の型そのものを
/// 生成物に書く必要をなくすため。EnumAsの各特殊化と違い、フィールドの実際のC++型が
/// NestedStructTagになることは無い=kFieldTagOfの通常の推論経路には乗らないタグのため)。
/// </summary>
inline constexpr uint8_t kFieldTagNestedStruct = kFieldTagOf<NestedStructTag>;

/// <summary>
/// フィールド1個分の Save/Load/Edit をまとめたインターフェース(旧 FieldTypeTag switch の
/// 置き換え)。呼び出し側(ToJsonViaDescriptor/FromJsonViaDescriptor/DrawComponentFieldsViaDescriptor)
/// はタグから引いたこのインターフェース越しにしか型を知らない。型を1つ増やすときの変更は
/// FieldTypeList への追加とここの特殊化(FieldStrategy.cpp)に閉じる。
///
/// Edit だけ ORIGINE_EDITOR_ENABLED で分岐しているのは、DrawComponentFieldsViaDescriptor 自体が
/// 同じマクロで丸ごと消える(editor/sceneEditor/ComponentDescriptorDrawer.h)ため。
/// OriGine.dll と ECS_TestGame.exe/ECS_TestEditor.exe は常に同じ構成(Debug/Develop/Release)で
/// 一緒にビルドされ、ORIGINE_EDITOR_ENABLED の定義有無は3構成すべてで揃っている
/// (CLAUDE.md/Phase4の前提)ため、この#ifdefがODR違反を起こすことはない。
/// </summary>
class IFieldStrategy {
public:
    virtual ~IFieldStrategy() = default;

    /// <summary>フィールドを JSON へ書き出す(_field.jsonKey_ をキーに使う)。</summary>
    virtual void Save(nlohmann::json& _outJson, const FieldDesc& _field, const void* _fieldPtr) const = 0;

    /// <summary>JSON の値(該当キーの中身)をフィールドへ書き戻す。</summary>
    virtual void Load(const nlohmann::json& _value, const FieldDesc& _field, void* _fieldPtr) const = 0;

#ifdef ORIGINE_EDITOR_ENABLED
    /// <returns>編集ウィジェットを描けたか(false ならサイズ不一致等で編集不可)</returns>
    virtual bool Edit(const FieldDesc& _field, const std::string& _label, void* _fieldPtr) const = 0;
#endif
};

/// <summary>
/// FieldTypeList の各具体型に対する Save/Load/Edit の実体(component/FieldStrategy.cpp)。
/// プライマリテンプレートはわざと定義を持たない: FieldTypeList に型を足したのに
/// この特殊化を書き忘れると、GetFieldStrategy の実装がこの不完全型をインスタンス化しようとして
/// コンパイルエラーになる。これが「対応を書き忘れたら気づける」ことの保証になっている
/// (要求: 書き忘れを実行時まで気づけない形にしない)。
/// </summary>
template <typename T>
class FieldStrategy;

/// <summary>
/// タグ値から対応するストラテジーを引く。DLL 内部専用(ORIGINE_API を付けない)。
/// 呼び出し元は ComponentReflection.cpp / ComponentDescriptorDrawer.cpp のみで、
/// どちらも同じ OriGine.dll に静的リンクされる(editor/** も OriGine プロジェクトに含まれる。
/// CLAUDE.md Phase4)ため、dllexport せずに通常の外部リンケージ関数として共有できる。
/// アプリ側(EXE)がこの表へ触る必要は無い(表経由シリアライズ/エディタ描画はどちらも
/// OriGine.dll の中で完結する)ため、あえて DLL 境界の外に出さない。
/// </summary>
/// <returns>範囲外のタグを渡した場合は nullptr(呼び出し側は黙って進めず理由をログに出すこと)</returns>
const IFieldStrategy* GetFieldStrategy(uint8_t _tag);

} // namespace OriGine
