#include "Entity.h"

void EntityRepository::Initialize() {
    entities_.resize(size_);
    entityActiveBits_.resize(size_);
    if (!uniqueEntityIDs_.empty()) {
        uniqueEntityIDs_.clear();
    }
}

void EntityRepository::Finalize() {
    entities_.clear();
    entityActiveBits_.resize(0);
    uniqueEntityIDs_.clear();
}
