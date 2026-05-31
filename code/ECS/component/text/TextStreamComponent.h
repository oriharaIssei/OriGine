#pragma once

#include "component/IComponent.h"

#include <cstddef>
#include <string>

namespace OriGine {

/// <summary>
/// 同じ Entity の TextComponent のテキストを、1 文字ずつ表示する
/// （タイプライター演出）ためのコンポーネント。
/// 全文は TextComponent.text が保持し、本コンポーネントは「何文字見せるか」を制御する。
/// 実際の反映（TextComponent.visibleCharCount の更新）は TextStreamSystem が行う。
/// </summary>
class TextStreamComponent
	: public IComponent {
	friend void to_json(nlohmann::json& j, const TextStreamComponent& c);
	friend void from_json(const nlohmann::json& j, TextStreamComponent& c);
public:
	TextStreamComponent()           = default;
	~TextStreamComponent() override = default;

	void Initialize(Scene* _scene, EntityHandle _owner) override;
	void Finalize() override;
	void Edit(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) override;

	/// <summary>再生（一時停止からの再開）。</summary>
	void Play() { playing = true; }
	/// <summary>一時停止。</summary>
	void Pause() { playing = false; }
	/// <summary>先頭に巻き戻す（次フレームから再度 1 文字ずつ表示）。</summary>
	void Reset();
	/// <summary>末尾まで一気に表示する。</summary>
	void Skip();

	bool IsFinished() const { return finished; }
	bool IsPlaying() const { return playing; }

	// --- 設定値（保存対象） ---
	float charsPerSecond = 30.0f;  ///< 1 秒あたりの表示文字数（コードポイント単位）
	float startDelay     = 0.0f;   ///< 表示開始までの遅延（秒）
	bool loop            = false;  ///< 完了後に先頭から繰り返す
	bool playing         = true;   ///< 再生中か

	// --- ランタイム状態（保存しない） ---
	float revealed     = 0.0f;          ///< これまでに表示したコードポイント数（小数）
	float elapsedDelay = 0.0f;          ///< 経過した開始遅延
	bool finished      = false;         ///< 末尾まで表示済みか
	int lastApplied    = -1;            ///< 直近で TextComponent に設定した visibleCharCount
	size_t textHash    = 0;             ///< TextComponent.text の変化検出用ハッシュ
};

} // namespace OriGine
