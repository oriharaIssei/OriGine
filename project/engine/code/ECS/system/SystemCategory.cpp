#include "SystemCategory.h"

using namespace OriGine;

/// <summary>
/// SystemCategoryを文字列に変換する
/// </summary>
/// <param name="_category">対象のカテゴリ</param>
/// <returns>カテゴリ名の文字列</returns>
std::string ToString(const SystemCategory& _category) {
    return kSystemCategoryString[static_cast<int>(_category)];
}
