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

		void Initialize() override;
		void Finalize() override;
	private:
		void UpdateEntity(EntityHandle _handle)override;

	};

} // namespace OriGine
