#pragma once

#include <cstdint>
#include <string>

namespace ReflectionCodeGen {

/// <summary>
/// FNV-1a 64bit。生成物を書き込む前に、既存ファイルの内容と比べて変化が無ければ
/// 書き込みをスキップする(タイムスタンプも更新しない)ための内容ハッシュに使う。
/// 暗号学的な強度は不要で、生成物という限られた入力に対する衝突耐性で十分なため
/// この程度の実装で足りる。
/// </summary>
inline uint64_t Fnv1a64(const std::string& _data) {
    uint64_t hash = 1469598103934665603ull;
    for (unsigned char c : _data) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ull;
    }
    return hash;
}

} // namespace ReflectionCodeGen
