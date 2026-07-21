// stb_rectpack / stb_truetype はヘッダオンリーライブラリのため、
// 実装本体を生成する翻訳単位をこの1ファイルに限定する（多重定義防止）。
#define STB_RECT_PACK_IMPLEMENTATION
#include "imgui/imstb_rectpack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "imgui/imstb_truetype.h"
