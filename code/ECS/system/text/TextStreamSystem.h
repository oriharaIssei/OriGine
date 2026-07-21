#pragma once

#include "system/ISystem.h"

namespace OriGine {

	class TextStreamComponent;

	/// <summary>
	/// TextStreamComponent を持つ Entity のテキストを、経過時間に応じて 1 文字ずつ
	/// 同 Entity の TextComponent に反映する（タイプライター演出）システム。
	/// ColliderRenderingSystem 同様、システムへのエンティティ登録に依存せず
	/// TextStreamComponent の配列を直接走査する。
	/// </summary>
	class TextStreamSystem
		: public ISystem{
	public:
		TextStreamSystem();
		~TextStreamSystem() override;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize() override;
		/// <summary>
		/// 終了処理
		/// </summary>
		void Finalize() override;
	private:
		/// <summary>
		/// 経過時間に応じて表示文字数を更新し、TextComponent に反映する
		/// </summary>
		/// <param name="_handle">対象エンティティ</param>
		void UpdateEntity(const EntityHandle& _handle)override;

	};

} // namespace OriGine
