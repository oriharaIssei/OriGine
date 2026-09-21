#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ReflectionCodeGen {

/// <summary>enum class NAME : underlying { ... }; から拾った情報。</summary>
struct EnumInfo {
    std::string name;
    std::string underlyingTypeText; // "uint8_t" 等。無指定なら "int"
};

/// <summary>
/// 1フィールド分の解析結果。typeSignature は宣言から拾った「型」部分のトークンを
/// 詰めた文字列(例: "Vec3f", "std::string", "IConstantBuffer<OutlineParamData>",
/// "Transform*")。このツールはもう typeSignature から FieldTypeTag(具体型の判定)を
/// 行わない(生成した .cpp の中で `kFieldTagOf<decltype(OriGine::Type::field)>` として
/// コンパイラに判定させる。Classifier.h のコメント参照)。typeSignature を全ファイル分
/// 読み終えたあとまで残しておくのは、enum の名前一致だけを見る FindEnumIndex のため
/// (同じファイル内で enum の宣言がフィールドより後に来る場合にも対応できるように)。
/// </summary>
struct FieldInfo {
    std::string name; // C++ 上のメンバ名
    std::string jsonKey; // 解決済みのJSONキー(既定規則 or ORIGINE_FIELD(json=...) 適用後)
    std::string typeSignature;
    // typeSignatureを構成する個々のトークン(空白で連結する前の生の文字列)。FindEnumIndexは
    // typeSignature全体の完全一致で足りるが、入れ子構造体の判定(FindNestedStructIndex)は
    // "IConstantBuffer<OutlineParamData>"のような型の中に構造体名がトークンとして
    // 含まれているかを見る必要があり、連結後の文字列に対する部分文字列一致では
    // 別の型名が偶然部分一致してしまう事故を防げないため、トークン単位で保持しておく。
    std::vector<std::string> typeTokens;
    bool noSave = false;
    // no_save(保存しない)とは独立したビット。エディタで灰色にする条件は本来こちら側の
    // 意味(システムが書き換える/計算し直す値かどうか)であって、保存するかどうかとは別の質問
    // なので、パーサ段階から別フィールドとして持つ(ドロワー側でnoSaveから読み替えない)。
    bool readOnly = false;
    int sourceLine = 0;
};

/// <summary>
/// ORIGINE_COMPONENT() または ORIGINE_STRUCT() が付いた1型分の解析結果。
/// 前者はコンポーネント(ComponentTypeIdを持つ)、後者は入れ子専用の構造体
/// (ComponentTypeIdを持たない)で、isComponentで区別する。
/// </summary>
struct TypeInfo {
    std::string name; // C++ のクラス/構造体名(例: "Transform")
    std::string headerIncludePath; // 生成 .cpp が #include するパス(例: "component/transform/Transform.h")
    std::vector<FieldInfo> fields;
    bool isComponent = true; // false: ORIGINE_STRUCT() が付いた入れ子専用の構造体
};

/// <summary>1ファイル分の解析結果。</summary>
struct FileParseResult {
    std::vector<TypeInfo> types; // このファイル内で ORIGINE_COMPONENT() が付いた型
    std::vector<EnumInfo> enums; // このファイル内で見つけた enum class 定義
};

/// <summary>
/// 分類済みのフィールド(Generator が使う最終形)。FieldTypeTag は持たない
/// (タグは生成した .cpp の中で kFieldTagOf<decltype(...)> としてコンパイラが決めるため、
/// このツール側で判定・保持する必要が無い)。enumIndex だけは、列挙型の名前一致という
/// コンパイラ側では引けない情報(同じ下地整数型を使う enum が複数あっても区別する必要がある)
/// なので、ここで確定させて Generator にそのまま渡す。
/// </summary>
struct ClassifiedField {
    std::string name;
    std::string jsonKey;
    int enumIndex = -1; // -1 なら enum ではない。0以上なら Generator.h の kEnums 配列への添字
    // -1 なら入れ子構造体ではない。0以上なら「ORIGINE_STRUCT()が付いた型だけを発見順に並べた
    // 一覧」への添字(Generatorが組む入れ子構造体専用TypeDesc表と同じ並び順。main.cppのFindNestedStructIndex参照)。
    int nestedStructIndex = -1;
    bool noSave = false;
    bool readOnly = false;
};

struct ClassifiedType {
    std::string name;
    std::string headerIncludePath;
    std::vector<ClassifiedField> fields;
    bool isComponent = true; // TypeInfo::isComponentと同じ意味
};

} // namespace ReflectionCodeGen
