#include "ECS/system/render/TextBoundsRenderSystem.h"

#include "Engine.h"
#include "directX12/DxDevice.h"
#include "winApp/WinApp.h"
#include "EngineConfig.h"
#include "text/FontManager.h"

#include <cstring>

namespace OriGine {

TextBoundsRenderSystem::TextBoundsRenderSystem() : BaseRenderSystem() {}
TextBoundsRenderSystem::~TextBoundsRenderSystem() = default;

void TextBoundsRenderSystem::Initialize() {
    BaseRenderSystem::Initialize();

    // 登録に依存せず走査するため TextComponent 配列を直接取得しておく
    textComponents_ = GetComponentArray<TextComponent>();

    WinApp* window = Engine::GetInstance()->GetWinApp();
    viewPortMat_   = MakeMatrix4x4::Orthographic(
        0, 0,
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight()),
        Config::Rendering::kMinDepth,
        Config::Camera::kDefaultFarClip);

    constBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    auto device = Engine::GetInstance()->GetDxDevice()->device_;
    vertexBuffer_.CreateBufferResource(device, maxVerts_ * sizeof(LineVertex));

    vbView_.BufferLocation = vertexBuffer_.GetResource()->GetGPUVirtualAddress();
    vbView_.SizeInBytes    = maxVerts_ * sizeof(LineVertex);
    vbView_.StrideInBytes  = sizeof(LineVertex);
}

void TextBoundsRenderSystem::Finalize() {
    constBuffer_.Finalize();
    vertexBuffer_.Finalize();

    BaseRenderSystem::Finalize();
}

void TextBoundsRenderSystem::PushLine(const Vec2f& _a, const Vec2f& _b, const Vec4f& _color) {
    lineVertices_.push_back({{_a[X], _a[Y], 0.0f, 1.0f}, _color});
    lineVertices_.push_back({{_b[X], _b[Y], 0.0f, 1.0f}, _color});
}

void TextBoundsRenderSystem::PushRect(float _x0, float _y0, float _x1, float _y1, const Vec4f& _color) {
    PushLine({_x0, _y0}, {_x1, _y0}, _color); // top
    PushLine({_x1, _y0}, {_x1, _y1}, _color); // right
    PushLine({_x1, _y1}, {_x0, _y1}, _color); // bottom
    PushLine({_x0, _y1}, {_x0, _y0}, _color); // left
}

void TextBoundsRenderSystem::Update() {
    // 描画対象（TextComponent）が無ければスキップ
    if (!textComponents_ || textComponents_->IsEmpty()) {
        return;
    }

    BuildLines();

    if (lineVertices_.empty()) {
        return;
    }

    Rendering();
}

void TextBoundsRenderSystem::BuildLines() {
    lineVertices_.clear();
    if (!textComponents_) {
        return;
    }

    FontManager* fm = FontManager::GetInstance();

    for (auto& slot : textComponents_->GetSlots()) {
        Entity* entity = GetEntity(slot.owner);
        if (!entity) {
            continue;
        }

        auto& texts = textComponents_->GetComponents(slot.owner);
        for (auto& text : texts) {
            if (!text.visible || !text.showBounds) continue;

            BitmapFont* font = fm->GetFont(text.fontHandle);
            if (!font) continue;

            // 毎フレーム再計算する。dirty は TextRenderSystem 側が消費するため、
            // ここでは消費しない（消費すると描画側の更新が止まる）。
            scratchLayout_.valid = false;
            layoutSystem_.UpdateLayout(*font, text, scratchLayout_, false);

            float x0 = text.position[X];
            float y0 = text.position[Y];
            float x1 = x0 + scratchLayout_.boundingSize[X];
            float y1 = y0 + scratchLayout_.boundingSize[Y];

            // レイアウト境界の枠
            PushRect(x0, y0, x1, y1, boundsColor_);

            // 折り返し幅のガイド（maxWidth が設定されているとき）
            if (text.maxWidth > 0.0f) {
                float wrapX = x0 + text.maxWidth;
                PushLine({wrapX, y0}, {wrapX, y1}, wrapColor_);
            }
        }
    }
}

void TextBoundsRenderSystem::UploadVertices() {
    uint32_t needed = static_cast<uint32_t>(lineVertices_.size());
    if (needed == 0) return;

    if (needed > maxVerts_) {
        vertexBuffer_.Finalize();
        maxVerts_   = needed * 2;
        auto device = Engine::GetInstance()->GetDxDevice()->device_;
        vertexBuffer_.CreateBufferResource(device, maxVerts_ * sizeof(LineVertex));

        vbView_.BufferLocation = vertexBuffer_.GetResource()->GetGPUVirtualAddress();
        vbView_.SizeInBytes    = maxVerts_ * sizeof(LineVertex);
        vbView_.StrideInBytes  = sizeof(LineVertex);
    }

    LineVertex* vtx = nullptr;
    vertexBuffer_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&vtx));
    std::memcpy(vtx, lineVertices_.data(), needed * sizeof(LineVertex));
    vertexBuffer_.GetResource()->Unmap(0, nullptr);
}

void TextBoundsRenderSystem::Rendering() {
    if (lineVertices_.empty()) return;

    WinApp* window = Engine::GetInstance()->GetWinApp();
    viewPortMat_   = MakeMatrix4x4::Orthographic(
        0, 0,
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight()),
        Config::Rendering::kMinDepth,
        Config::Camera::kDefaultFarClip);

    constBuffer_->vpMat = viewPortMat_;
    constBuffer_.ConvertToBuffer();

    UploadVertices();

    auto& cmdList = dxCommand_->GetCommandList();

    StartRender();

    cmdList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    cmdList->SetPipelineState(pso_->pipelineState.Get());

    constBuffer_.SetForRootParameter(cmdList, 0);

    cmdList->IASetVertexBuffers(0, 1, &vbView_);
    cmdList->DrawInstanced(static_cast<UINT>(lineVertices_.size()), 1, 0, 0);

    lineVertices_.clear();
}

void TextBoundsRenderSystem::StartRender() {
    auto& cmdList = dxCommand_->GetCommandList();
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}

bool TextBoundsRenderSystem::ShouldSkipRender() const {
    return lineVertices_.empty();
}

void TextBoundsRenderSystem::CreatePSO() {
    ShaderManager* sm = ShaderManager::GetInstance();

    if (sm->IsRegisteredPipelineStateObj("Line2D_Normal")) {
        pso_ = sm->GetPipelineStateObj("Line2D_Normal");
        return;
    }

    sm->LoadShader("Line2D.VS");
    sm->LoadShader("Line2D.PS", kShaderDirectory, L"ps_6_0");

    ShaderInformation info{};
    info.vsKey = "Line2D.VS";
    info.psKey = "Line2D.PS";

    D3D12_ROOT_PARAMETER rootParam = {};
    rootParam.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam.ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParam.Descriptor.ShaderRegister = 0;
    info.pushBackRootParameter(rootParam);

    D3D12_INPUT_ELEMENT_DESC inputDescs[2] = {};
    inputDescs[0].SemanticName      = "POSITION";
    inputDescs[0].Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    info.pushBackInputElementDesc(inputDescs[0]);

    inputDescs[1].SemanticName      = "COLOR";
    inputDescs[1].Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    info.pushBackInputElementDesc(inputDescs[1]);

    info.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    info.changeCullMode(D3D12_CULL_MODE_NONE);
    info.blendMode_ = BlendMode::Normal;

    pso_ = sm->CreatePso("Line2D_Normal", info, Engine::GetInstance()->GetDxDevice()->device_);
}

} // namespace OriGine
