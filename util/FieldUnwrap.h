#pragma once

/// stl
#include <cstddef>

namespace OriGine {

/// <summary>
/// 型ディスクリプタ生成(tools/ReflectionCodeGen)が「フィールドの実データがどこにあるか」を
/// 汎用的に扱うためのトレイト。既定(このプライマリテンプレート)は「宣言されたフィールドの型
/// そのものが実データ」であることを表し、Type = T, kOffset = 0 になる。
///
/// IConstantBuffer&lt;X&gt;(directX12/buffer/IConstantBuffer.h)のように「宣言された型 T とは
/// 別の場所(内部の openData_)に実データがある」型だけがこのテンプレートを特殊化し、そこだけ
/// Type/kOffset を書き換える。生成ツールはこの特殊化の存在(=IConstantBufferという名前)を
/// 知らない。`decltype(Owner::field)` から `FieldUnwrap<...>::Type` / `::kOffset` を引くだけの
/// 機械的な式を出力するので、特殊化は実データを持つ型自身の定義側に書けばよく、生成ツールに
/// 個々の「実データをラップする型」の名前を教える必要が無い。
///
/// util/ 直下に置いてあるのは、directX12/buffer(GPU寄りの下位層)から
/// ECS/component(型ディスクリプタ)へ逆依存させないため。このヘッダ自体は
/// cstddef 以外に依存しない、どちらの層からも安全に参照できるだけの小さなトレイトにしてある。
/// </summary>
template <typename T>
struct FieldUnwrap {
    using Type                      = T;
    static constexpr size_t kOffset = 0;
};

} // namespace OriGine
