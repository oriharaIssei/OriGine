#pragma once

///stl
#include <memory>
#include <string>
///engine
//assets
#include "animation/Animation.h"
//directX12
#include "directX12/Mesh.h"

enum class SilhouetteType {
    Plane,
    Sphere,
    Circle,
    Cylinder,
};

class ISilhouette {
public:
    ISilhouette(SilhouetteType _type);
    virtual ~ISilhouette();

    void Initialize();
    void Finalize();

    virtual void Create() = 0;
    virtual void Update() = 0;
    virtual void Draw()   = 0;

protected:
    std::unique_ptr<TextureMesh> meshBuff_;

private:
    SilhouetteType type_;
};
