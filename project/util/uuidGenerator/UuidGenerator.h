#pragma once

/// externals
#include <uuid/uuid.h>

/// <summary>
/// Externalsにある uuidライブラリの生成ラッパー
/// </summary>
class UuidGenerator {
public:
    /// <summary>
    /// ランダムなUUIDを生成する
    /// </summary>
    /// <returns></returns>
    static uuids::uuid RandomGenerate();
};
