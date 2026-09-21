#include "Classifier.h"

namespace ReflectionCodeGen {

int FindEnumIndex(const std::string& _typeSignature, const std::vector<EnumInfo>& _enums) {
    for (size_t i = 0; i < _enums.size(); ++i) {
        if (_enums[i].name == _typeSignature) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace ReflectionCodeGen
