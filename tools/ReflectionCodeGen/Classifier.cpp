#include "Classifier.h"

#include "Tokenizer.h" // ParseError(曖昧な入れ子構造体一致を検出したときに使う)

namespace ReflectionCodeGen {

int FindEnumIndex(const std::string& _typeSignature, const std::vector<EnumInfo>& _enums) {
    for (size_t i = 0; i < _enums.size(); ++i) {
        if (_enums[i].name == _typeSignature) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int FindNestedStructIndex(const std::vector<std::string>& _typeTokens, const std::vector<std::string>& _structNames,
    const std::string& _fileName, int _sourceLine) {
    int found = -1;
    for (size_t i = 0; i < _structNames.size(); ++i) {
        bool matches = false;
        for (const auto& token : _typeTokens) {
            if (token == _structNames[i]) {
                matches = true;
                break;
            }
        }
        if (!matches) {
            continue;
        }
        if (found >= 0) {
            throw ParseError(_fileName, _sourceLine,
                "フィールドの型に複数の入れ子構造体名('" + _structNames[found] + "', '" + _structNames[i] +
                    "')がトークンとして同時に現れた。曖昧なため対応していない");
        }
        found = static_cast<int>(i);
    }
    return found;
}

} // namespace ReflectionCodeGen
