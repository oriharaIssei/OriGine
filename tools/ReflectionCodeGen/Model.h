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
    bool noSave = false;
    int sourceLine = 0;
};

/// <summary>ORIGINE_COMPONENT() が付いた1型分の解析結果。</summary>
struct TypeInfo {
    std::string name; // C++ のクラス/構造体名(例: "Transform")
    std::string headerIncludePath; // 生成 .cpp が #include するパス(例: "component/transform/Transform.h")
    std::vector<FieldInfo> fields;
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
    bool noSave = false;
};

struct ClassifiedType {
    std::string name;
    std::string headerIncludePath;
    std::vector<ClassifiedField> fields;
};

} // namespace ReflectionCodeGen
