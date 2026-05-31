#pragma once

#include "component/IComponent.h"
#include "text/FontTypes.h"

#include <string>
#include <vector>
#include <Vector2.h>
#include <Vector4.h>

namespace OriGine {

enum class TextAlign : uint8_t {
	Left,
	Center,
	Right,
};

class TextComponent
	: public IComponent {
	friend void to_json(nlohmann::json& j,const TextComponent& c);
	friend void from_json(const nlohmann::json& j,TextComponent& c);
public:
	TextComponent() = default;
	~TextComponent() override = default;

	void Initialize(Scene* _scene, EntityHandle _owner) override;
	void Finalize() override;
	void Edit(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) override;

	std::string text;
	Vec2f position = {0.0f, 0.0f};
	Vec4f color = {1.0f, 1.0f, 1.0f, 1.0f};
	FontHandle fontHandle = kInvalidFontHandle;
	float fontSize = 24.0f;
	float maxWidth = 0.0f;
	float lineSpacing = 1.0f;   ///< 行間（行の高さに対する倍率）
	float charSpacing = 0.0f;   ///< 字間（各文字の送り幅に加算するピクセル量、負値で詰める）
	TextAlign align = TextAlign::Left;
	int32_t renderPriority = 10;
	int32_t visibleCharCount = -1; ///< 表示するコードポイント数。-1 で全文表示（TextStreamSystem が制御）
	bool dirty = true;
	bool visible = true;
	bool showBounds = false; ///< TextBoundsRenderSystem でレイアウト枠をデバッグ描画するか
};

struct TextLayoutResult {
	std::vector<GlyphQuad> quads;
	Vec2f boundingSize = {0.0f, 0.0f};
	bool valid = false;
};

} // namespace OriGine
