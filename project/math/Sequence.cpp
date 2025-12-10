#include "Sequence.h"

using namespace OriGine;

std::string std::to_string(CommonSequenceType _type) {
    switch (_type) {
    case CommonSequenceType::Arithmetic:
        return "Arithmetic";
    case CommonSequenceType::Geometric:
        return "Geometric";
    default:
        return "Unknown";
    }
}
