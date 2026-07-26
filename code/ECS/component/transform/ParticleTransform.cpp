#include "ParticleTransform.h"

using namespace OriGine;

/// <summary>
/// パーティクル1粒分のワールド行列とUV行列を再計算する。
///
/// Affineは スケール → 回転 → 平行移動 の順に合成した行列を返す。
/// この順序でないと、回転や平行移動の後にスケールが掛かって形が歪んだり、
/// 移動量までスケール倍されたりしてしまう。
///
/// 親行列は右から掛ける（worldMat *= parent）。この行列規約では
/// 「ローカル変換 → 親の変換」の順に適用されるため、子の変換を先に済ませてから
/// 親の座標系へ持ち上げる形になる。
/// parentWorldMat が null のときは、そのエミッタがワールド直下に配置されていることを意味する
/// </summary>
void ParticleTransform::UpdateMatrix() {
    worldMat = MakeMatrix4x4::Affine(scale, rotate, translate);
    uvMat    = MakeMatrix4x4::Affine(uvScale, uvRotate, uvTranslate);

    if (parentWorldMat) {
        worldMat *= *parentWorldMat;
    }
}

/// <summary>
/// ParticleTransformの状態をJSONへ書き出す。
/// worldMat/uvMatは毎フレームUpdateMatrixで再計算される派生値なので保存しない
/// </summary>
/// <param name="_j">書き込み先のJSON</param>
/// <param name="_comp">シリアライズ対象のParticleTransform</param>
void OriGine::to_json(nlohmann::json& _j, const ParticleTransform& _comp) {
    _j = nlohmann::json{
        {"scale", _comp.scale},
        {"rotate", _comp.rotate},
        {"translate", _comp.translate},
        {"uvScale", _comp.uvScale},
        {"uvRotate", _comp.uvRotate},
        {"uvTranslate", _comp.uvTranslate},
        {"color", _comp.color}};
}
/// <summary>
/// JSONからParticleTransformの状態を復元する
/// </summary>
/// <param name="_j">読み込み元のJSON</param>
/// <param name="_comp">復元先のParticleTransform</param>
void OriGine::from_json(const nlohmann::json& _j, ParticleTransform& _comp) {
    _j.at("scale").get_to(_comp.scale);
    _j.at("rotate").get_to(_comp.rotate);
    _j.at("translate").get_to(_comp.translate);
    _j.at("uvScale").get_to(_comp.uvScale);
    _j.at("uvRotate").get_to(_comp.uvRotate);
    _j.at("uvTranslate").get_to(_comp.uvTranslate);
    _j.at("color").get_to(_comp.color);
}
