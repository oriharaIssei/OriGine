#include "UuidGenerator.h"

uuids::uuid UuidGenerator::RandomGenerate() {
    static std::mt19937 rng{std::random_device{}()};
    static uuids::uuid_random_generator gen{rng};
    return gen();
}
