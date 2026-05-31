#pragma once

#include "system/render/base/BaseRenderSystem.h"
#include "ECS/system/text/TextLayoutSystem.h"
#include "ECS/component/text/TextComponent.h"
#include "text/FontTypes.h"

#include "directX12/DxCommand.h"
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"
#include "directX12/ShaderManager.h"
#include "Matrix4x4.h"

#include <unordered_map>
#include <vector>

namespace OriGine {

class TextRenderSystem
	: public BaseRenderSystem {
public:
	TextRenderSystem();
	~TextRenderSystem() override;

	void Initialize() override;
	void Finalize() override;

	TextLayoutSystem& GetLayoutSystem() { return layoutSystem_; }

protected:
	void CreatePSO() override;
	void StartRender() override;
	void Rendering() override;
	void DispatchRenderer(EntityHandle _entity) override;
	bool ShouldSkipRender() const override;

private:
	/// <summary>
	/// フォント 1 つ分の GPU アトラステクスチャ。
	/// </summary>
	struct FontAtlasGpu {
		DxResource resource;
		DxSrvDescriptor srv;
		bool created = false;
		int uploadedWidth = 0;
		int uploadedHeight = 0;
	};

	FontAtlasGpu& EnsureAtlas(BitmapFont* _font);
	void CreateAtlasTexture(BitmapFont& _font, FontAtlasGpu& _gpu);
	void ReuploadAtlasTexture(BitmapFont& _font, FontAtlasGpu& _gpu);
	void RebuildVertexBuffer();

	TextLayoutSystem layoutSystem_;

	struct RenderEntry {
		int32_t priority;
		BitmapFont* font;
		TextLayoutResult* layout;
	};
	std::vector<RenderEntry> renderEntries_;
	std::unordered_map<uuids::uuid, TextLayoutResult> layoutCache_;

	// フォント（BitmapFont*）ごとの GPU アトラス
	std::unordered_map<BitmapFont*, FontAtlasGpu> atlases_;

	DxResource vertexBuffer_;
	DxResource indexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	uint32_t quadCount_ = 0;
	uint32_t maxQuads_ = 1024;

	PipelineStateObj* pso_ = nullptr;
	Matrix4x4 viewPortMat_;

	struct TextConstBuffer {
		Matrix4x4 vpMat = MakeMatrix4x4::Identity();

		struct ConstantBuffer {
			Matrix4x4 vpMat;
			ConstantBuffer& operator=(const TextConstBuffer& _src) {
				vpMat = _src.vpMat;
				return *this;
			}
		};
	};
	IConstantBuffer<TextConstBuffer> constBuffer_;
};

} // namespace OriGine
