#include "Classifier.h"

namespace ReflectionCodeGen {

FieldTypeTag ClassifyFieldType(const std::string& _typeSignature, const std::vector<EnumInfo>& _enums, size_t& _outEnumIndex) {
    _outEnumIndex = static_cast<size_t>(-1);

    if (_typeSignature == "bool") return FieldTypeTag::Bool;
    if (_typeSignature == "int32_t" || _typeSignature == "int") return FieldTypeTag::Int32;
    if (_typeSignature == "uint32_t") return FieldTypeTag::UInt32;
    if (_typeSignature == "float") return FieldTypeTag::Float;
    if (_typeSignature == "Vec2f") return FieldTypeTag::Vec2f;
    if (_typeSignature == "Vec3f") return FieldTypeTag::Vec3f;
    if (_typeSignature == "Vec4f") return FieldTypeTag::Vec4f;
    if (_typeSignature == "Quaternion") return FieldTypeTag::Quaternion;
    if (_typeSignature == "Matrix4x4") return FieldTypeTag::Matrix4x4;
    if (_typeSignature == "std::string") return FieldTypeTag::String;

    for (size_t i = 0; i < _enums.size(); ++i) {
        if (_enums[i].name == _typeSignature) {
            _outEnumIndex = i;
            return FieldTypeTag::Enum;
        }
    }

    // ここに来るもの(例: Matrix3x3, ComponentHandle, FontHandle, IConstantBuffer<...>,
    // size_t, ポインタ型)はすべて Opaque。D3 の決定どおり「具体型に当てはまらないものは
    // まとめて Opaque」というキャッチオールで、中身までは覗かない。
    return FieldTypeTag::Opaque;
}

} // namespace ReflectionCodeGen
