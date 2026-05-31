#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// engine
#include "ECS/system/text/TextLayoutSystem.h"
#include "ECS/component/text/TextComponent.h"
#include "text/FontTypes.h"

#include "directX12/DxCommand.h"
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxResource.h"
#include "directX12/ShaderManager.h"
#include "Matrix4x4.h"

#include <vector>

namespace OriGine {

/// <summary>
/// TextComponent のレイアウト境界（表示枠）をライン描画でデバッグ可視化するシステム。
/// ColliderRenderingSystem 同様、システムへのエンティティ登録に依存せず、
/// TextComponent の配列を直接走査して毎フレーム動的にラインを生成・描画する。
/// showBounds が有効な TextComponent のみ対象とする。
/// </summary>
class TextBoundsRenderSystem
    : public BaseRenderSystem {
public:
    TextBoundsRenderSystem();
    ~TextBoundsRenderSystem() override;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePSO() override;
    void StartRender() override;
    void Rendering() override;
    bool ShouldSkipRender() const override;

private:
    void BuildLines();
    void PushLine(const Vec2f& _a, const Vec2f& _b, const Vec4f& _color);
    void PushRect(float _x0, float _y0, float _x1, float _y1, const Vec4f& _color);
    void UploadVertices();

    struct LineVertex {
        Vec4f pos;
        Vec4f color;
    };
    std::vector<LineVertex> lineVertices_;

    TextLayoutSystem layoutSystem_;
    TextLayoutResult scratchLayout_; ///< 毎フレーム使い回す一時レイアウト結果

    // 登録に依存せず直接走査するための TextComponent 配列
    ComponentArray<TextComponent>* textComponents_ = nullptr;

    DxResource vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    uint32_t maxVerts_ = 1024;

    PipelineStateObj* pso_ = nullptr;
    Matrix4x4 viewPortMat_;

    struct BoundsConstBuffer {
        Matrix4x4 vpMat = MakeMatrix4x4::Identity();

        struct ConstantBuffer {
            Matrix4x4 vpMat;
            ConstantBuffer& operator=(const BoundsConstBuffer& _src) {
                vpMat = _src.vpMat;
                return *this;
            }
        };
    };
    IConstantBuffer<BoundsConstBuffer> constBuffer_;

    Vec4f boundsColor_ = {0.2f, 1.0f, 0.3f, 1.0f}; ///< 表示枠の色
    Vec4f wrapColor_   = {1.0f, 0.6f, 0.1f, 0.8f}; ///< 折り返し幅ガイドの色
};

} // namespace OriGine
