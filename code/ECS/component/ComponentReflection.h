#pragma once

/// stl
#include <cstdint>

/// externals
#include <nlohmann/json.hpp>

/// ECS
#include "component/ComponentTypeId.h"

/// DLL境界
#include "OriGineApi.h"

namespace OriGine {

/// <summary>FieldDesc::flags_ のビット。保存対象から外すフィールドに立てる(D4)。</summary>
inline constexpr uint8_t kFieldFlagNoSave = 1u << 0;

/// <summary>
/// FieldDesc::flags_ のビット。エディタで編集不可(灰色表示)にするフィールドに立てる。
/// no_save とは独立: no_save は「保存するかどうか」、read_only は「エディタで書き換えて良いか」
/// という別の質問で、片方だけが真になる組み合わせがある(例: OutlineComponent::paramData は
/// no_save だが read_only ではない = 保存はしないが編集はできる、入れ子の中身を反映するため)。
/// </summary>
inline constexpr uint8_t kFieldFlagReadOnly = 1u << 1;

/// <summary>FieldDesc::enumIndex_ の無効値(Enum 以外のフィールドはこの値を持つ)。</summary>
inline constexpr uint16_t kInvalidEnumIndex = 0xFFFFu;

/// <summary>
/// 列挙型1つ分の情報(D3)。列挙値の名前一覧はまだ持たない
/// (値名まで反映する仕組みはエディタを作り直すときに足す。docs/plans/phase-03c-design.md 決定3)。
/// </summary>
struct EnumDesc {
    const char* name_; // 列挙型自身の名前(例: "TextAlign")
    uint32_t underlyingSize_; // 下地の整数型のバイト数(sizeof(std::underlying_type_t<T>) 相当)
};

/// <summary>
/// フィールド1行分の情報。全型のフィールドを1本の静的配列に並べて使う(D2)ための
/// 固定長レコード。冷たい経路(起動時・保存・エディタ)でしか読まないが、将来プロセスを
/// 分離したときにそのまま転送できるよう、フラットな POD に保っている。
/// レイアウト(合計32バイト): 名前8 / JSONキー8 / オフセット4 / サイズ4 / 型タグ1 / フラグ1 /
/// 列挙表への添字2 / 詰め物4。
/// </summary>
struct FieldDesc {
    const char* name_; // C++ 上のメンバ名
    const char* jsonKey_; // 保存時に使うキー名(注釈が無ければメンバ名から末尾の '_' を除いた名前)
    uint32_t offset_; // オーナー型の先頭からのバイトオフセット(offsetof)
    uint32_t size_; // フィールド自身のバイト数(sizeof)
    uint8_t typeTag_; // component/FieldStrategy.h の kFieldTagOf<T> が返す値(GetFieldStrategy() への添字)
    uint8_t flags_; // kFieldFlagNoSave 等
    uint16_t enumIndex_; // typeTag_ == kFieldTagEnum のときだけ有効。EnumDesc 表への添字
    // typeTag_ == kFieldTagNestedStruct(component/FieldStrategy.h)のときだけ有効。
    // GetNestedTypeTable() への添字(入れ子の注釈付き構造体、ORIGINE_STRUCT() の TypeDesc を指す)。
    // それ以外の型では常に0(旧 reserved_ と同じ「32バイトへの詰め物」の役割を兼ねる)。
    uint32_t nestedTypeIndex_;
};
static_assert(sizeof(FieldDesc) == 32,
    "FieldDesc は32バイト1行の表として設計されている(docs/plans/phase-03c-design.md D1/D2)。"
    "レイアウトを変えるときはこの表のバイト数を数え直すこと。");

/// <summary>
/// 型1つ分の情報。フィールドは個別配列を持たず、共有の FieldDesc 配列への
/// (開始位置, 個数)で指す(D2)。
/// レイアウト(合計24バイト): 型名8 / フィールド開始位置4 / フィールド数4 / 型のサイズ4 / 型ID4。
/// </summary>
struct TypeDesc {
    const char* typeName_; // nameof<T>() と同じ規則の型名(D6: シリアライズの型名キーと合わせる)
    uint32_t fieldStart_; // 共有 FieldDesc 配列内での開始位置
    uint32_t fieldCount_; // フィールド数
    uint32_t typeSize_; // sizeof(T)
    uint32_t typeId_; // ComponentTypeId(登録時に埋める。未登録中は kInvalidComponentTypeId)
};
static_assert(sizeof(TypeDesc) == 24,
    "TypeDesc は24バイト1行の表として設計されている(docs/plans/phase-03c-design.md D1/D2)。"
    "レイアウトを変えるときはこの表のバイト数を数え直すこと。");

/// <summary>
/// 型IDを添字にした固定配列でディスクリプタを引く(D6)。ComponentRepository の
/// componentArrays_ と同じ「添字 = 型ID、要素数 kMaxComponentTypes」の不変条件に揃えてある。
/// 実体(生成された TypeDesc の集合)は ComponentReflection.cpp が保持し、
/// 生成コードの RegisterGeneratedComponentDescriptors() が登録時に埋める。
/// </summary>
/// <returns>登録済みなら対応する TypeDesc へのポインタ、未登録なら nullptr</returns>
ORIGINE_API const TypeDesc* GetTypeDescriptor(uint32_t _typeId);

/// <summary>
/// 型IDに対応するディスクリプタを登録する。生成コードからのみ呼ばれる想定
/// (静的初期化子による自己登録は静的ライブラリでリンカに捨てられるため、
/// RegisterUsingComponents() から明示的に呼ぶ設計になっている。Q16)。
/// </summary>
/// <param name="_typeId">ComponentTypeId(kMaxComponentTypes 未満であること)</param>
/// <param name="_desc">登録する TypeDesc(生成コード側が所有する静的ストレージを指す)</param>
ORIGINE_API void RegisterTypeDescriptor(uint32_t _typeId, const TypeDesc* _desc);

/// <summary>
/// 全型のフィールドを1本にまとめた共有配列(D2)を登録する。生成コードが1回だけ呼ぶ想定
/// (今は生成 .cpp が1本なので呼び出しも1回だが、将来複数の生成単位に分かれても
/// 対応できるよう、呼ぶたびに置き換えるのではなく先頭からの通し番号で追記できる形にはせず、
/// 単純な「最後に登録した表を使う」にしてある。複数登録が必要になったら再設計する)。
/// </summary>
/// <param name="_fields">生成コードが持つ静的な FieldDesc 配列の先頭</param>
/// <param name="_count">配列の要素数</param>
ORIGINE_API void RegisterFieldTable(const FieldDesc* _fields, uint32_t _count);

/// <summary>共有 FieldDesc 配列の先頭を取得する(未登録なら nullptr)。</summary>
ORIGINE_API const FieldDesc* GetFieldTable();

/// <summary>共有 FieldDesc 配列の要素数を取得する。</summary>
ORIGINE_API uint32_t GetFieldTableCount();

/// <summary>
/// 入れ子の注釈付き構造体(ORIGINE_STRUCT())専用の TypeDesc 表を登録する。コンポーネント用の
/// 表(g_typeDescriptors、型IDを添字にした kMaxComponentTypes 要素の固定配列)とは別に持つ理由:
/// 入れ子構造体は ComponentTypeId を持たない(コンポーネントではないため、その配列に混ぜられない)。
/// RegisterFieldTable と同じ「生成コードが1回だけ呼ぶ、最後に登録した表を使う」流儀にしてある。
/// </summary>
/// <param name="_types">生成コードが持つ静的な TypeDesc 配列の先頭(並び順が FieldDesc::nestedTypeIndex_ の意味を決める)</param>
/// <param name="_count">配列の要素数</param>
ORIGINE_API void RegisterNestedTypeTable(const TypeDesc* _types, uint32_t _count);

/// <summary>入れ子構造体専用 TypeDesc 表の先頭を取得する(未登録なら nullptr)。</summary>
ORIGINE_API const TypeDesc* GetNestedTypeTable();

/// <summary>入れ子構造体専用 TypeDesc 表の要素数を取得する。</summary>
ORIGINE_API uint32_t GetNestedTypeTableCount();

/// <summary>
/// この型のシリアライズ(ComponentArray の Save/Load 系)をディスクリプタ表経由にするかどうかの
/// 型ごとの切り替え(Phase 3D-1)。既定は false(従来どおり手書きの ADL to_json/from_json を使う)。
///
/// 「型ID→ディスクリプタが登録されていて、かつ保存対象フィールドに Opaque が無い」を満たす型を
/// 自動的に true へ倒さない理由: その条件を満たしていても、型ごとの事情(例: SmoothingEffectParam の
/// to_json/from_json が `namespace OriGine` の外に定義されている既知のバグにより、ADL 経由の
/// シリアライズが元々機能していない)で手書きのままにしておきたい場合があるため。
/// このバグを true へのオプトインで自動的に踏み越えて「直って見える」ことを避けるため、
/// 対象は型ごとに明示的な特殊化でオプトインする方式にしてある。
/// 各コンポーネントのヘッダ側で、`ORIGINE_COMPONENT()` を付けた型の定義の直後に
/// `template <> inline constexpr bool kUsesDescriptorSerialization<Foo> = true;` として特殊化する
/// (対象ヘッダは IComponent.h 経由でこのファイルを既に include している)。
/// </summary>
template <typename ComponentType>
inline constexpr bool kUsesDescriptorSerialization = false;

/// <summary>
/// 型ディスクリプタ(TypeDesc/FieldDesc)を使って、保存対象のフィールドを JSON へ書き出す。
/// `kFieldFlagNoSave` が立つフィールドは書かない。Opaque なフィールドは中身を復元する情報が
/// 表に無いため書き出せない(呼び出し側で `kUsesDescriptorSerialization` を true にする型は
/// 保存対象に Opaque を含めないこと。含めてしまった場合はログを出してそのフィールドだけ飛ばす、
/// 検出用の安全網)。
/// </summary>
/// <param name="_outJson">書き込み先(型名キーやコンポーネント間で共有する "Handle" は呼び出し側が付ける)</param>
/// <param name="_obj">シリアライズ対象オブジェクトの先頭アドレス</param>
/// <param name="_desc">対象型の TypeDesc</param>
ORIGINE_API void ToJsonViaDescriptor(nlohmann::json& _outJson, const void* _obj, const TypeDesc& _desc);

/// <summary>
/// 型ディスクリプタを使って、JSON からフィールドを読み込む。キーが無いフィールドや
/// `kFieldFlagNoSave` が立つフィールドには一切触れない(呼び出し前にデフォルト構築済みの値を
/// そのまま残す。手書きの from_json が `contains()` で守っている挙動と同じにするため)。
/// </summary>
/// <param name="_inJson">読み込み元(型ごとの1コンポーネント分のオブジェクト)</param>
/// <param name="_obj">書き込み先オブジェクトの先頭アドレス(呼び出し前にデフォルト構築済みであること)</param>
/// <param name="_desc">対象型の TypeDesc</param>
ORIGINE_API void FromJsonViaDescriptor(const nlohmann::json& _inJson, void* _obj, const TypeDesc& _desc);

} // namespace OriGine

/// ==========================================================================
/// 型ディスクリプタ生成ツール(project/engine/tools/ReflectionCodeGen)が読み取る注釈。
/// 展開結果はどちらも空(コンパイラにとっては何もしない)。ツールは実際の C++
/// プリプロセッサを通さず、ヘッダのテキストをそのまま読んでこれらの呼び出しを検出するため、
/// 必ず1文として(末尾に ';' を付けて)独立した行に書くこと。
///
///   ORIGINE_COMPONENT();
///     型に1つだけ付ける印。この型の public データメンバを反映対象にする
///     (可視性に印は無く、public 全部が既定で対象。docs/plans/phase-03c-design.md 決定4/8)。
///
///   ORIGINE_FIELD(no_save);
///   ORIGINE_FIELD(read_only);
///   ORIGINE_FIELD(json = "customKey");
///   ORIGINE_FIELD(json = "customKey", no_save);
///   ORIGINE_FIELD(no_save, read_only);
///     直後の1フィールド宣言だけに効く例外指定。json= は保存キー名の上書き、no_save は
///     保存対象から外す指定、read_only はエディタで編集不可(灰色表示)にする指定
///     (既定は保存する・編集できる・キーはメンバ名から末尾の '_' を除いた名前)。
///     no_save と read_only は独立したビットで、併記できる。知らない引数は生成ツールが
///     エラーで止める。
///
///   ORIGINE_STRUCT();
///     コンポーネントではない、入れ子専用の構造体に付ける印(ORIGINE_COMPONENT()の代わりに
///     1つだけ付ける。両方付けるとエラーで止まる)。この構造体を指すフィールドが
///     コンポーネント側にあると、生成ツールはフィールドの型トークンにこの構造体の名前が
///     含まれることを見て「入れ子フィールド」として扱い、実行時に FieldStrategy.h の
///     NestedStructTag ストラテジー経由でこの構造体自身の TypeDesc を再帰的にたどって
///     Save/Load/Edit する(スイッチ文を増やさずに済ませるため)。
/// ==========================================================================
#define ORIGINE_COMPONENT()
#define ORIGINE_STRUCT()
#define ORIGINE_FIELD(...)
