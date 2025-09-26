#include "Orientation.h"

Orientation Orientation::Identity() {
    Orientation orientation;
    orientation.rot     = Quaternion::Identity();
    orientation.axis[0] = Vec3f(1, 0, 0);
    orientation.axis[1] = Vec3f(0, 1, 0);
    orientation.axis[2] = Vec3f(0, 0, 1);
    return orientation;
}

void Orientation::UpdateAxes() {
    rot         = rot.normalize();

    Matrix4x4 m = MakeMatrix::RotateQuaternion(rot);
    axis[0]     = Vec3(m[0][0], m[1][0], m[2][0]).normalize();
    axis[1]     = Vec3(m[0][1], m[1][1], m[2][1]).normalize();
    axis[2]     = Vec3(m[0][2], m[1][2], m[2][2]).normalize();
}
