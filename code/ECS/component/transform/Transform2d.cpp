#include "Transform2d.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

/// <summary>
/// ローカルの変換行列を更新
/// </summary>
void Transform2d::UpdateMatrix() {
    // Scale -> Rotate -> Translate の順で合成する。
    // 各行列は原点基準の変換として作られるため、この順で掛けることで
    // 「原点でスケール・回転してから、最後に平行移動で目的の位置へ動かす」という
    // 見た目通りの変換になる(順序を変えると回転軸や拡縮の基準点がずれてしまう)
    worldMat = MakeMatrix3x3::Scale(scale)
               * MakeMatrix3x3::Rotate(rotate)
               * MakeMatrix3x3::Translate(translate);

    // 親の影響を適用
    // 行ベクトル規約のため、自分のローカル行列(左)に親のワールド行列(右)を掛けることで
    // ローカル座標 -> 親空間 -> ワールド空間、の順に変換が伝播する
    if (parent) {
        worldMat *= parent->worldMat;
    }
}

/// <summary>
/// エディタ表示
/// </summary>
void Transform2d::Edit(Scene* /*_scene*/, const EntityHandle& /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    DragGuiVectorCommand("Scale##" + _parentLabel, scale, 0.01f);
    DragGuiCommand("Rotate##" + _parentLabel, rotate, 0.01f);
    DragGuiVectorCommand("Translate##" + _parentLabel, translate, 0.01f);
#endif // _DEBUG
}
