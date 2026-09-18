#include "component/ComponentReflection.h"

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

} // namespace OriGine
