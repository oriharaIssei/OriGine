#include "component/ComponentReflection.h"

/// stl
#include <cstddef>
#include <cstdint>

/// ECS: FieldTypeTag switch を置き換えたストラテジー表(D3→ストラテジーパターン化)。
#include "component/FieldStrategy.h"

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

        const IFieldStrategy* strategy = GetFieldStrategy(f.typeTag_);
        if (!strategy) {
            // GetFieldStrategy側で理由をログ済み。ここでは黙って次のフィールドへ進む
            // (1フィールド壊れても残りは書き出す。計測器と同じ「黙って全部落とさない」原則)。
            continue;
        }
        const void* fieldPtr = reinterpret_cast<const std::byte*>(_obj) + f.offset_;
        strategy->Save(_outJson, f, fieldPtr);
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

        const IFieldStrategy* strategy = GetFieldStrategy(f.typeTag_);
        if (!strategy) {
            continue;
        }
        void* fieldPtr = reinterpret_cast<std::byte*>(_obj) + f.offset_;
        strategy->Load(_inJson.at(f.jsonKey_), f, fieldPtr);
    }
}

} // namespace OriGine
