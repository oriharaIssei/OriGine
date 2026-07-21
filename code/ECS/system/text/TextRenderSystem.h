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

/// <summary>
/// TextComponent を持つエンティティを描画するレンダリングシステム。
/// フォントのグリフアトラスをGPUテクスチャとして管理し、レイアウト結果を頂点バッファへ書き出して描画する。
/// </summary>
class TextRenderSystem
	: public BaseRenderSystem {
public:
	TextRenderSystem();
	~TextRenderSystem() override;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize() override;

	/// <summary>
	/// テキストレイアウト計算に使用するシステムを取得する
	/// </summary>
	TextLayoutSystem& GetLayoutSystem() { return layoutSystem_; }

protected:
	/// <summary>
	/// PSO作成
	/// </summary>
	void CreatePSO() override;
	/// <summary>
	/// レンダリング開始処理
	/// </summary>
	void StartRender() override;
	/// <summary>
	/// レンダリング処理
	/// </summary>
	void Rendering() override;
	/// <summary>
	/// 描画対象コンポーネントの割り当て
	/// </summary>
	/// <param name="_entity">エンティティハンドル</param>
	void DispatchRenderer(const EntityHandle& _entity) override;
	/// <summary>
	/// レンダリングをスキップするかどうか
	/// </summary>
	/// <returns>true = スキップする / false = スキップしない</returns>
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

	/// <summary>
	/// 指定フォントのGPUアトラスを取得する。未生成なら生成し、内容が更新されていれば再アップロードする。
	/// </summary>
	/// <param name="_font">対象のフォント</param>
	/// <returns>対応するGPUアトラスへの参照</returns>
	FontAtlasGpu& EnsureAtlas(BitmapFont* _font);
	/// <summary>
	/// フォントのグリフアトラスからGPUテクスチャを新規作成し、アップロードする。
	/// </summary>
	void CreateAtlasTexture(BitmapFont& _font, FontAtlasGpu& _gpu);
	/// <summary>
	/// 既存のGPUアトラステクスチャへ、更新されたグリフアトラスの内容を再アップロードする。
	/// サイズが変化している場合はテクスチャを作り直す。
	/// </summary>
	void ReuploadAtlasTexture(BitmapFont& _font, FontAtlasGpu& _gpu);
	/// <summary>
	/// renderEntries_ のレイアウト結果から頂点バッファを構築する。
	/// 必要クアッド数が maxQuads_ を超える場合はバッファを拡張して作り直す。
	/// </summary>
	void RebuildVertexBuffer();

	TextLayoutSystem layoutSystem_; // テキストレイアウト計算用システム

	/// <summary>
	/// 1テキストコンポーネント分の描画情報（優先度・フォント・レイアウト結果）
	/// </summary>
	struct RenderEntry {
		int32_t priority; // 描画優先度
		BitmapFont* font; // 使用するフォント
		TextLayoutResult* layout; // レイアウト計算済みのグリフクアッド列
	};
	std::vector<RenderEntry> renderEntries_; // 今回の更新で描画対象となったエントリ一覧
	std::unordered_map<uuids::uuid, TextLayoutResult> layoutCache_; // エンティティごとのレイアウト計算結果キャッシュ

	// フォント（BitmapFont*）ごとの GPU アトラス
	std::unordered_map<BitmapFont*, FontAtlasGpu> atlases_;

	DxResource vertexBuffer_; // 全テキスト共有の頂点バッファ
	DxResource indexBuffer_; // 全テキスト共有のインデックスバッファ
	D3D12_VERTEX_BUFFER_VIEW vbView_{}; // 頂点バッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView_{}; // インデックスバッファビュー
	uint32_t quadCount_ = 0; // 今回の描画で使用するクアッド数
	uint32_t maxQuads_ = 1024; // 頂点/インデックスバッファが確保済みの最大クアッド数

	PipelineStateObj* pso_ = nullptr; // テキスト描画用PSO
	Matrix4x4 viewPortMat_; // ビューポート（正射影）行列

	/// <summary>
	/// テキスト描画用の定数バッファデータ
	/// </summary>
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
	IConstantBuffer<TextConstBuffer> constBuffer_; // ビュープロジェクション行列を保持する定数バッファ
};

} // namespace OriGine
