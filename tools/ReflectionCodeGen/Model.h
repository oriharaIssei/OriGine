#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ReflectionCodeGen {

/// <summary>フィールドの値の種類。OriGine::FieldTypeTag と1対1で対応させること。</summary>
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

/// <summary>enum class NAME : underlying { ... }; から拾った情報。</summary>
struct EnumInfo {
    std::string name;
    std::string underlyingTypeText; // "uint8_t" 等。無指定なら "int"
};

/// <summary>
/// 1フィールド分の解析結果。typeSignature は宣言から拾った「型」部分のトークンを
/// 詰めた文字列(例: "Vec3f", "std::string", "IConstantBuffer<OutlineParamData>",
/// "Transform*")で、FieldTypeTag への分類は全ファイルを読み終えて enum registry が
/// 揃ってから Classifier.h が行う(同じファイル内で enum の宣言がフィールドより後に
/// 来る場合にも対応できるようにするため)。
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

/// <summary>分類済みのフィールド(Generator が使う最終形)。</summary>
struct ClassifiedField {
    std::string name;
    std::string jsonKey;
    FieldTypeTag typeTag = FieldTypeTag::Opaque;
    std::string enumName; // typeTag == Enum のときだけ使う
    bool noSave = false;
};

struct ClassifiedType {
    std::string name;
    std::string headerIncludePath;
    std::vector<ClassifiedField> fields;
};

} // namespace ReflectionCodeGen
