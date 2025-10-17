#include "PrimitiveType.h"

std::string std::to_string(PrimitiveType _type) {
    switch (_type) {
    case PrimitiveType::Plane:
        return "Plane";
    case PrimitiveType::Ring:
        return "Ring";
    // case PrimitiveType::Circle:
    //     return "Circle";
    case PrimitiveType::Box:
        return "Box";
    // case PrimitiveType::Sphere:
    //     return "Sphere";
    // case PrimitiveType::Torus:
    //     return "Torus";
    // case PrimitiveType::Cylinder:
    //     return "Cylinder";
    // case PrimitiveType::Cone:
    //     return "Cone";
    default:
        return "Unknown";
    }
}
