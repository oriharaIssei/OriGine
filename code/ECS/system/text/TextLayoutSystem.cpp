#include "ECS/system/text/TextLayoutSystem.h"

namespace OriGine {

namespace {

/// <summary>
/// UTF-8 文字列の先頭1文字をUnicodeコードポイントにデコードする。
/// </summary>
/// <param name="_str">デコード対象の文字列の先頭ポインタ</param>
/// <param name="_len">_str から読み取り可能な残りバイト数</param>
/// <param name="_outBytes">消費したバイト数の出力先</param>
/// <returns>デコードされたコードポイント（不正な場合は 0xFFFD）</returns>
uint32_t DecodeUtf8(const char* _str, size_t _len, size_t& _outBytes) {
	if (_len == 0) { _outBytes = 0; return 0; }

	uint8_t c = static_cast<uint8_t>(_str[0]);
	if (c < 0x80) {
		_outBytes = 1;
		return c;
	} else if ((c & 0xE0) == 0xC0 && _len >= 2) {
		_outBytes = 2;
		return ((c & 0x1F) << 6) | (static_cast<uint8_t>(_str[1]) & 0x3F);
	} else if ((c & 0xF0) == 0xE0 && _len >= 3) {
		_outBytes = 3;
		return ((c & 0x0F) << 12) |
			   ((static_cast<uint8_t>(_str[1]) & 0x3F) << 6) |
			   (static_cast<uint8_t>(_str[2]) & 0x3F);
	} else if ((c & 0xF8) == 0xF0 && _len >= 4) {
		_outBytes = 4;
		return ((c & 0x07) << 18) |
			   ((static_cast<uint8_t>(_str[1]) & 0x3F) << 12) |
			   ((static_cast<uint8_t>(_str[2]) & 0x3F) << 6) |
			   (static_cast<uint8_t>(_str[3]) & 0x3F);
	}
	_outBytes = 1;
	return 0xFFFD;
}

/// <summary>
/// 指定コードポイントがCJK（日本語・中国語・韓国語）文字かどうかを判定する。
/// </summary>
bool IsCjk(uint32_t _cp) {
	return (_cp >= 0x3000 && _cp <= 0x9FFF) ||
		   (_cp >= 0xF900 && _cp <= 0xFAFF) ||
		   (_cp >= 0xFF00 && _cp <= 0xFFEF);
}

/// <summary>
/// 指定コードポイントの直前で行折り返しが可能かどうかを判定する。
/// </summary>
bool CanBreakBefore(uint32_t _cp) {
	if (IsCjk(_cp)) return true;
	if (_cp == ' ' || _cp == '\t') return true;
	return false;
}

} // namespace

/// <summary>
/// テキストコンポーネントのレイアウトを計算し、グリフクアッド列を生成する。
/// </summary>
/// <param name="_font">使用するフォント（必要なグリフがあれば追加生成される）</param>
/// <param name="_text">対象のテキストコンポーネント</param>
/// <param name="_result">レイアウト結果の出力先</param>
/// <param name="_consumeDirty">true なら計算後に TextComponent::dirty をクリアする</param>
void TextLayoutSystem::UpdateLayout(BitmapFont& _font, TextComponent& _text, TextLayoutResult& _result, bool _consumeDirty) {
	if (!_text.dirty && _result.valid) return;

	_result.quads.clear();

	if (_font.IsTrueType()) {
		_font.EnsureGlyphs(_text.text.c_str(), _text.text.size());
	}

	float scale = 1.0f;
	if (_font.IsTrueType() && _font.GetFontSize() > 0.0f) {
		scale = _text.fontSize / _font.GetFontSize();
	}

	float cursorX = _text.position[X];
	float cursorY = _text.position[Y];
	float startX = cursorX;
	float maxX = cursorX;
	float lh = _font.GetLineHeight() * scale * _text.lineSpacing;
	float wrapWidth = _text.maxWidth;

	const char* str = _text.text.c_str();
	size_t len = _text.text.size();

	struct LineInfo {
		size_t quadStart;
		size_t quadCount;
		float width;
	};
	std::vector<LineInfo> lines;
	lines.push_back({0, 0, 0.0f});

	// 字間は「文字の前」に挿入する（行頭の文字には付けない）。
	// こうすることで行末・折り返し・改行のいずれでも余分な字間が残らない。
	bool firstGlyphOnLine = true;

	// 表示するコードポイント数（-1 で全文）。タイプライター演出で先頭 N 文字だけ描画する。
	int shownCp = 0;

	size_t i = 0;
	while (i < len) {
		size_t bytes = 0;
		uint32_t cp = DecodeUtf8(str + i, len - i, bytes);
		i += bytes;

		// visibleCharCount に達したら以降は描画しない
		if (_text.visibleCharCount >= 0 && shownCp >= _text.visibleCharCount) {
			break;
		}
		++shownCp;

		if (cp == '\n') {
			lines.back().width = cursorX - startX;
			if (cursorX > maxX) maxX = cursorX;
			cursorX = startX;
			cursorY += lh;
			lines.push_back({_result.quads.size(), 0, 0.0f});
			firstGlyphOnLine = true;
			continue;
		}
		if (cp == '\r') continue;

		const GlyphMetrics& g = _font.GetGlyph(cp);

		float glyphAdvance = g.advance * scale;
		// 行頭以外では文字の前に字間（charSpacing）を入れる
		float gap = firstGlyphOnLine ? 0.0f : _text.charSpacing;

		if (wrapWidth > 0.0f && (cursorX - startX + gap + glyphAdvance) > wrapWidth) {
			if (cursorX > startX) {
				lines.back().width = cursorX - startX;
				if (cursorX > maxX) maxX = cursorX;
				cursorX = startX;
				cursorY += lh;
				lines.push_back({_result.quads.size(), 0, 0.0f});
				firstGlyphOnLine = true;
				gap = 0.0f; // 折り返し後は行頭なので字間なし
			}
		}

		cursorX += gap;

		if (cp != ' ' && g.size[X] > 0.0f && g.size[Y] > 0.0f) {
			GlyphQuad quad;
			quad.posMin = {cursorX + g.bearing[X] * scale, cursorY + g.bearing[Y] * scale};
			quad.posMax = {quad.posMin[X] + g.size[X] * scale, quad.posMin[Y] + g.size[Y] * scale};
			quad.uvMin = g.uvMin;
			quad.uvMax = g.uvMax;
			quad.color = _text.color;
			_result.quads.push_back(quad);
			lines.back().quadCount++;
		}

		cursorX += glyphAdvance;
		if (cursorX > maxX) maxX = cursorX;
		firstGlyphOnLine = false;
	}

	lines.back().width = cursorX - startX;

	// 左揃え以外の場合、行ごとの幅に応じて各グリフクアッドをX方向にオフセットする
	if (_text.align != TextAlign::Left) {
		float totalWidth = (wrapWidth > 0.0f) ? wrapWidth : (maxX - startX);
		for (auto& line : lines) {
			float offset = 0.0f;
			if (_text.align == TextAlign::Center) {
				offset = (totalWidth - line.width) * 0.5f;
			} else if (_text.align == TextAlign::Right) {
				offset = totalWidth - line.width;
			}
			if (offset > 0.0f) {
				for (size_t qi = line.quadStart; qi < line.quadStart + line.quadCount; ++qi) {
					_result.quads[qi].posMin[X] += offset;
					_result.quads[qi].posMax[X] += offset;
				}
			}
		}
	}

	_result.boundingSize = Vec2f(maxX - startX, cursorY + lh - _text.position[Y]);
	_result.valid = true;
	if (_consumeDirty) {
		_text.dirty = false;
	}
}

} // namespace OriGine
