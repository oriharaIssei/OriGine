#pragma once

/// externals
#include <uuid/uuid.h>

/// DLL境界
#include "OriGineApi.h"

/// <summary>
/// Externalsにある uuidライブラリの生成ラッパー
/// </summary>
class ORIGINE_API UuidGenerator {
public:
    /// <summary>
    /// ランダムなUUIDを生成する
    /// </summary>
    /// <returns></returns>
    static uuids::uuid RandomGenerate();
};
