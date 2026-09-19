#pragma once

/// stl
#include <cstdint>

/// externals
#include <nlohmann/json.hpp>

/// ECS
#include "component/ComponentTypeId.h"

namespace OriGine {

/// <summary>
/// フィールドの値の種類を表す閉じた列挙(Phase 3C D3)。
/// int32 に畳まない理由: エディタでは列挙型は Combo、数値は Slider / Input / Drag と
/// ウィジェットが変わるため、値の種類ごとに区別できる形で持つ。コンテナ・入れ子コンポーネント・
/// GPU バッファ等、ここに挙げた具体型に当てはまらないものはすべて Opaque にまとめる
/// (今は「保存しない/カスタム扱い」の目印として使うだけで、中身までは覗かない)。
/// </summary>
enum class FieldTypeTag : uint8_t {
    Bool,
    Int32,
    UInt32,
    Float,
    Vec2f,
    Vec3f,
    Vec4f,
    Quaternion,
    Matrix4x4,
    String,
    Enum,
    Opaque,
};

/// <summary>FieldDesc::flags_ のビット。保存対象から外すフィールドに立てる(D4)。</summary>
inline constexpr uint8_t kFieldFlagNoSave = 1u << 0;

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
    uint8_t typeTag_; // FieldTypeTag の値
    uint8_t flags_; // kFieldFlagNoSave 等
    uint16_t enumIndex_; // typeTag_ == Enum のときだけ有効。EnumDesc 表への添字
    uint32_t reserved_; // 32バイトへの詰め物。将来の属性追加用に予約(常に0)
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
const TypeDesc* GetTypeDescriptor(uint32_t _typeId);

/// <summary>
/// 型IDに対応するディスクリプタを登録する。生成コードからのみ呼ばれる想定
/// (静的初期化子による自己登録は静的ライブラリでリンカに捨てられるため、
/// RegisterUsingComponents() から明示的に呼ぶ設計になっている。Q16)。
/// </summary>
/// <param name="_typeId">ComponentTypeId(kMaxComponentTypes 未満であること)</param>
/// <param name="_desc">登録する TypeDesc(生成コード側が所有する静的ストレージを指す)</param>
void RegisterTypeDescriptor(uint32_t _typeId, const TypeDesc* _desc);

/// <summary>
/// 全型のフィールドを1本にまとめた共有配列(D2)を登録する。生成コードが1回だけ呼ぶ想定
/// (今は生成 .cpp が1本なので呼び出しも1回だが、将来複数の生成単位に分かれても
/// 対応できるよう、呼ぶたびに置き換えるのではなく先頭からの通し番号で追記できる形にはせず、
/// 単純な「最後に登録した表を使う」にしてある。複数登録が必要になったら再設計する)。
/// </summary>
/// <param name="_fields">生成コードが持つ静的な FieldDesc 配列の先頭</param>
/// <param name="_count">配列の要素数</param>
void RegisterFieldTable(const FieldDesc* _fields, uint32_t _count);

/// <summary>共有 FieldDesc 配列の先頭を取得する(未登録なら nullptr)。</summary>
const FieldDesc* GetFieldTable();

/// <summary>共有 FieldDesc 配列の要素数を取得する。</summary>
uint32_t GetFieldTableCount();

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
void ToJsonViaDescriptor(nlohmann::json& _outJson, const void* _obj, const TypeDesc& _desc);

/// <summary>
/// 型ディスクリプタを使って、JSON からフィールドを読み込む。キーが無いフィールドや
/// `kFieldFlagNoSave` が立つフィールドには一切触れない(呼び出し前にデフォルト構築済みの値を
/// そのまま残す。手書きの from_json が `contains()` で守っている挙動と同じにするため)。
/// </summary>
/// <param name="_inJson">読み込み元(型ごとの1コンポーネント分のオブジェクト)</param>
/// <param name="_obj">書き込み先オブジェクトの先頭アドレス(呼び出し前にデフォルト構築済みであること)</param>
/// <param name="_desc">対象型の TypeDesc</param>
void FromJsonViaDescriptor(const nlohmann::json& _inJson, void* _obj, const TypeDesc& _desc);

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
///   ORIGINE_FIELD(json = "customKey");
///   ORIGINE_FIELD(json = "customKey", no_save);
///     直後の1フィールド宣言だけに効く例外指定。json= は保存キー名の上書き、no_save は
///     保存対象から外す指定(既定は保存する・キーはメンバ名から末尾の '_' を除いた名前)。
/// ==========================================================================
#define ORIGINE_COMPONENT()
#define ORIGINE_FIELD(...)
