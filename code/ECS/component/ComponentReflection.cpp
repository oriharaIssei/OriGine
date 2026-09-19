#include "component/ComponentReflection.h"

/// stl
#include <cstddef>
#include <cstdint>
#include <string>

/// math(表経由シリアライズが扱う具体型。FieldTypeTag の閉じた列挙と1対1に対応する)
#include "math/Quaternion.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

/// logger
#include "logger/Logger.h"

namespace OriGine {

namespace {
// 型IDを添字にした固定配列(D6)。componentArrays_ 等と同じ「添字 = 型ID」の
// 不変条件に揃えている。要素はポインタなので、未登録の型は nullptr のままでよい
// (「ID はあるがディスクリプタがない」型を自然に表せる。Q1a/Q1d)。
const TypeDesc* g_typeDescriptors[kMaxComponentTypes] = {};

const FieldDesc* g_fieldTable      = nullptr;
uint32_t g_fieldTableCount = 0;

/// <summary>_obj + _offset の位置を ValueType& として読む(FieldDesc::offset_ はバイト単位)。</summary>
template <typename ValueType>
const ValueType& FieldAs(const void* _obj, uint32_t _offset) {
    return *reinterpret_cast<const ValueType*>(reinterpret_cast<const std::byte*>(_obj) + _offset);
}

/// <summary>FieldAs の書き込み版。</summary>
template <typename ValueType>
ValueType& FieldAsMut(void* _obj, uint32_t _offset) {
    return *reinterpret_cast<ValueType*>(reinterpret_cast<std::byte*>(_obj) + _offset);
}

/// <summary>
/// Enum フィールドを下地のバイト数(FieldDesc::size_)に応じた符号無し整数として読み出す。
/// 符号付き enum の下地型は今のところ表に情報が無い(EnumDesc は名前とバイト数しか持たない。
/// docs/plans/phase-03c-design.md D3)ため、符号無しとして扱う。対象10型の唯一の Enum
/// (TextComponent::align, uint8_t 下地)は符号無しなのでここでは実害が無いが、将来 int 下地の
/// enum を表経由にする場合はこの制約に注意すること。
/// </summary>
uint64_t ReadEnumAsUInt(const void* _obj, uint32_t _offset, uint32_t _size) {
    switch (_size) {
    case 1:
        return FieldAs<uint8_t>(_obj, _offset);
    case 2:
        return FieldAs<uint16_t>(_obj, _offset);
    case 4:
        return FieldAs<uint32_t>(_obj, _offset);
    case 8:
        return FieldAs<uint64_t>(_obj, _offset);
    default:
        LOG_ERROR("ReadEnumAsUInt: 未対応の enum サイズ({}バイト)", _size);
        return 0;
    }
}

/// <summary>ReadEnumAsUInt の書き込み版。</summary>
void WriteEnumFromUInt(void* _obj, uint32_t _offset, uint32_t _size, uint64_t _value) {
    switch (_size) {
    case 1:
        FieldAsMut<uint8_t>(_obj, _offset) = static_cast<uint8_t>(_value);
        break;
    case 2:
        FieldAsMut<uint16_t>(_obj, _offset) = static_cast<uint16_t>(_value);
        break;
    case 4:
        FieldAsMut<uint32_t>(_obj, _offset) = static_cast<uint32_t>(_value);
        break;
    case 8:
        FieldAsMut<uint64_t>(_obj, _offset) = static_cast<uint64_t>(_value);
        break;
    default:
        LOG_ERROR("WriteEnumFromUInt: 未対応の enum サイズ({}バイト)", _size);
        break;
    }
}
} // namespace

const TypeDesc* GetTypeDescriptor(uint32_t _typeId) {
    if (_typeId >= kMaxComponentTypes) {
        return nullptr;
    }
    return g_typeDescriptors[_typeId];
}

void RegisterTypeDescriptor(uint32_t _typeId, const TypeDesc* _desc) {
    if (_typeId >= kMaxComponentTypes) {
        return;
    }
    g_typeDescriptors[_typeId] = _desc;
}

void RegisterFieldTable(const FieldDesc* _fields, uint32_t _count) {
    g_fieldTable      = _fields;
    g_fieldTableCount = _count;
}

const FieldDesc* GetFieldTable() {
    return g_fieldTable;
}

uint32_t GetFieldTableCount() {
    return g_fieldTableCount;
}

void ToJsonViaDescriptor(nlohmann::json& _outJson, const void* _obj, const TypeDesc& _desc) {
    const FieldDesc* fields = GetFieldTable();
    for (uint32_t i = 0; i < _desc.fieldCount_; ++i) {
        const FieldDesc& f = fields[_desc.fieldStart_ + i];
        if (f.flags_ & kFieldFlagNoSave) {
            continue;
        }

        switch (static_cast<FieldTypeTag>(f.typeTag_)) {
        case FieldTypeTag::Bool:
            _outJson[f.jsonKey_] = FieldAs<bool>(_obj, f.offset_);
            break;
        case FieldTypeTag::Int32:
            _outJson[f.jsonKey_] = FieldAs<int32_t>(_obj, f.offset_);
            break;
        case FieldTypeTag::UInt32:
            _outJson[f.jsonKey_] = FieldAs<uint32_t>(_obj, f.offset_);
            break;
        case FieldTypeTag::Float:
            _outJson[f.jsonKey_] = FieldAs<float>(_obj, f.offset_);
            break;
        case FieldTypeTag::Vec2f:
            _outJson[f.jsonKey_] = FieldAs<Vec2f>(_obj, f.offset_);
            break;
        case FieldTypeTag::Vec3f:
            _outJson[f.jsonKey_] = FieldAs<Vec3f>(_obj, f.offset_);
            break;
        case FieldTypeTag::Vec4f:
            _outJson[f.jsonKey_] = FieldAs<Vec4f>(_obj, f.offset_);
            break;
        case FieldTypeTag::Quaternion:
            _outJson[f.jsonKey_] = FieldAs<OriGine::Quaternion>(_obj, f.offset_);
            break;
        case FieldTypeTag::String:
            _outJson[f.jsonKey_] = FieldAs<std::string>(_obj, f.offset_);
            break;
        case FieldTypeTag::Enum:
            _outJson[f.jsonKey_] = ReadEnumAsUInt(_obj, f.offset_, f.size_);
            break;
        case FieldTypeTag::Matrix4x4:
        case FieldTypeTag::Opaque:
        default:
            // kUsesDescriptorSerialization<T> をtrueにする型の選定で、保存対象(NoSaveでない)
            // フィールドにOpaque/Matrix4x4を含めてしまった場合の安全網。黙って欠落させず、
            // どのフィールドが書けなかったかをログに残す(計測基盤と同じ「黙って落とさない」原則)。
            LOG_ERROR("ToJsonViaDescriptor: フィールド '{}' は表経由で保存できない型タグ({})なのに保存対象になっている",
                f.name_, f.typeTag_);
            break;
        }
    }
}

void FromJsonViaDescriptor(const nlohmann::json& _inJson, void* _obj, const TypeDesc& _desc) {
    const FieldDesc* fields = GetFieldTable();
    for (uint32_t i = 0; i < _desc.fieldCount_; ++i) {
        const FieldDesc& f = fields[_desc.fieldStart_ + i];
        if (f.flags_ & kFieldFlagNoSave) {
            continue;
        }
        if (!_inJson.contains(f.jsonKey_)) {
            // 6章の罠8番: キーが無いフィールドは触らず、デフォルト構築済みの値をそのまま残す
            // (手書きの from_json が contains() で守っている挙動と合わせる)。
            continue;
        }

        const nlohmann::json& value = _inJson.at(f.jsonKey_);
        switch (static_cast<FieldTypeTag>(f.typeTag_)) {
        case FieldTypeTag::Bool:
            value.get_to(FieldAsMut<bool>(_obj, f.offset_));
            break;
        case FieldTypeTag::Int32:
            value.get_to(FieldAsMut<int32_t>(_obj, f.offset_));
            break;
        case FieldTypeTag::UInt32:
            value.get_to(FieldAsMut<uint32_t>(_obj, f.offset_));
            break;
        case FieldTypeTag::Float:
            value.get_to(FieldAsMut<float>(_obj, f.offset_));
            break;
        case FieldTypeTag::Vec2f:
            value.get_to(FieldAsMut<Vec2f>(_obj, f.offset_));
            break;
        case FieldTypeTag::Vec3f:
            value.get_to(FieldAsMut<Vec3f>(_obj, f.offset_));
            break;
        case FieldTypeTag::Vec4f:
            value.get_to(FieldAsMut<Vec4f>(_obj, f.offset_));
            break;
        case FieldTypeTag::Quaternion:
            value.get_to(FieldAsMut<OriGine::Quaternion>(_obj, f.offset_));
            break;
        case FieldTypeTag::String:
            value.get_to(FieldAsMut<std::string>(_obj, f.offset_));
            break;
        case FieldTypeTag::Enum: {
            uint64_t raw = 0;
            value.get_to(raw);
            WriteEnumFromUInt(_obj, f.offset_, f.size_, raw);
            break;
        }
        case FieldTypeTag::Matrix4x4:
        case FieldTypeTag::Opaque:
        default:
            LOG_ERROR("FromJsonViaDescriptor: フィールド '{}' は表経由で読み込めない型タグ({})なのに保存対象になっている",
                f.name_, f.typeTag_);
            break;
        }
    }
}

} // namespace OriGine
