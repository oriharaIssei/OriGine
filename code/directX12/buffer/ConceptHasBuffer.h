#pragma once

/// stl
#include <concepts>
#include <type_traits>

namespace OriGine {

/// <summary>
/// 内部宣言で ConstantBuffer 型を持つことを要求するコンセプト
/// </summary>
/// <remarks>
/// SimpleConstantBuffer&lt;T&gt; 等は GPU にマップした T::ConstantBuffer 領域へ
/// `*mappingData_ = 入力データ` という代入で書き込む設計になっているため、
/// テンプレート引数 T が「ConstantBuffer という入れ物の型を持ち、かつ T から
/// その ConstantBuffer へ代入できる」ことをコンパイル時に強制する。
/// これにより、対応する定数バッファ型を定義し忘れたクラスをテンプレート引数に
/// 渡した場合、実行時ではなくコンパイル時にエラーとして検出できる。
/// </remarks>
template <typename T>
concept HasInConstantBuffer = requires {
    typename T::ConstantBuffer;
    requires ::std::is_copy_assignable_v<typename T::ConstantBuffer>;
    { ::std::declval<typename T::ConstantBuffer>() = ::std::declval<const T&>() } -> ::std::same_as<typename T::ConstantBuffer&>;
};

}
