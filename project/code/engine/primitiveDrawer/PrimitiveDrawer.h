#pragma once

#include <Engine.h>

#include "directX12/ShaderManager.h"

#include <directX12/PipelineStateObj.h>

#include "directX12/IConstantBuffer.h"
#include "directX12/Mesh.h"
#include "material/Material.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

#include "directX12/DxCommand.h"

#include <array>

#include <memory>
#include <wrl.h>

#include <stdint.h>
#include <Vector3.h>

class PrimitiveDrawer {
public:
    static void Init();
    static void Finalize();

    static void Line(const Vec3f& p0,
        const Vec3f& p1,
        const IConstantBuffer<Transform>& Transform,
        const IConstantBuffer<Material>* material);

    /*static void Triangle(const Vec3f& p0,
                         const Vec3f& p1,
                         const Vec3f& p2,
                         const IConstantBuffer<Transform>& Transform,
                         const IConstantBuffer<CameraTransform>& viewProj,
                         const IConstantBuffer<Material>* material);
    static void Quad(const Vec3f& p0,
                     const Vec3f& p1,
                     const Vec3f& p2,
                     const Vec3f& p3,
                     const IConstantBuffer<Transform>& Transform,
                     const IConstantBuffer<CameraTransform>& viewProj,
                     const IConstantBuffer<Material>* material);*/

    static void PreDrawLine();
    static void PostDrawLine();
    static void ResetInstanceVal() {
        lineInstanceVal_ = 0;
        // triangleInstanceVal_ = 0;
        // quadInstanceVal_ = 0;
    };

private:
    static void CreatePso(Engine* system = Engine::getInstance());

private:
    static std::unique_ptr<DxCommand> dxCommand_;

    static std::array<PipelineStateObj*, kBlendNum> linePso_;
    static std::array<std::string, kBlendNum> linePsoKeys_;

    static std::unique_ptr<PrimitiveMesh> lineMesh_;
    static uint32_t lineInstanceVal_;

    static BlendMode currentBlendMode_;

public:
    static void setBlendMode(BlendMode blend) {
        currentBlendMode_ = blend;
    }

    // static PipelineStateObj* getPrimitivePso(BlendMode blend){ return trianglePso_[static_cast<size_t>(blend)]; }

    // static const std::array<std::string,kBlendNum>& getTrianglePsoKeys(){ return trianglePsoKeys_; }
    static const std::array<std::string, kBlendNum>& getLinePsoKeys() { return linePsoKeys_; }
};
