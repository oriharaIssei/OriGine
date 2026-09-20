#pragma once

#ifdef ORIGINE_EDITOR_ENABLED

/// stl
#include <string>

namespace OriGine {
struct TypeDesc;
}

namespace OriGine {

/// <summary>
/// 型ディスクリプタ(TypeDesc/FieldDesc, component/ComponentReflection.h)を辿って、
/// コンポーネント1個ぶんのフィールドを ImGui のウィジェットとして描画・編集する汎用ドロワー
/// (Phase 3【Claude】"汎用 ImGui ドロワー"。EntityInspector から使う)。
///
/// なぜ型を知らずに安全に読み書きできるのか:
///   FieldDesc::offset_ はコード生成時(tools/ReflectionCodeGen)に offsetof(具象型, メンバ) で
///   確定した値であり、1つの TypeDesc は必ず特定の1つの具象型に対して生成されている
///   (ComponentReflection.h の TypeDesc のコメントを参照)。したがって _obj が _desc の指す型と
///   一致するアドレスでありさえすれば、offsetof の値をそのまま足して読み書きしても安全。
///   型の取り違えが起きるとしたら「呼び出し側が別の型の TypeDesc を渡してしまった」場合だけなので、
///   その照合(型IDと型名が一致しているか)は呼び出し側(EntityComponentRegion)の責務にしてある。
///
///   もう一段下の照合として、FieldDesc::typeTag_ が期待する C++ 型の sizeof と FieldDesc::size_ が
///   食い違っていないかを各フィールドごとに見ている(generated ファイルを手で壊した場合の最後の砦)。
///   オフセット経由の書き込みはサイズを取り違えると隣のフィールドまで破壊するため、ここだけは
///   黙って進めず、書き込まずに理由を表示する。
/// </summary>
/// <param name="_obj">対象コンポーネントの先頭アドレス(_desc が指す具象型と一致していること。呼び出し側の責務)</param>
/// <param name="_desc">対象コンポーネントの型ディスクリプタ</param>
/// <param name="_idSuffix">ImGui の ID 衝突を避けるための識別子(エンティティ・コンポーネント単位で一意な文字列を渡すこと)</param>
void DrawComponentFieldsViaDescriptor(void* _obj, const TypeDesc& _desc, const std::string& _idSuffix);

} // namespace OriGine

#endif // ORIGINE_EDITOR_ENABLED
