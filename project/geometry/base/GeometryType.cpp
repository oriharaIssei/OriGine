#include "GeometryType.h"

std::string std::to_string(geometry::Type _type) {
    switch (_type) {
    case geometry::Type::Plane:
        return "Plane";
    case geometry::Type::Ring:
        return "Ring";
    // case geometry::Type::Circle:
    //     return "Circle";
    case geometry::Type::Box:
        return "Box";
    // case geometry::Type::Sphere:
    //     return "Sphere";
    // case geometry::Type::Torus:
    //     return "Torus";
    // case geometry::Type::Cylinder:
    //     return "Cylinder";
    // case geometry::Type::Cone:
    //     return "Cone";
    default:
        return "Unknown";
    }
}
