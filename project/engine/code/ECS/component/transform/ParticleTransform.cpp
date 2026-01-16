#include "ParticleTransform.h"

using namespace OriGine;

void ParticleTransform::UpdateMatrix(){
	worldMat = MakeMatrix4x4::Affine(scale,rotate,translate);
	uvMat    = MakeMatrix4x4::Affine(uvScale,uvRotate,uvTranslate);

	if(parentWorldMat){
        worldMat *= *parentWorldMat;
	}
}

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
void OriGine::from_json(const nlohmann::json& _j, ParticleTransform& _comp) {
    _j.at("scale").get_to(_comp.scale);
    _j.at("rotate").get_to(_comp.rotate);
    _j.at("translate").get_to(_comp.translate);
    _j.at("uvScale").get_to(_comp.uvScale);
    _j.at("uvRotate").get_to(_comp.uvRotate);
    _j.at("uvTranslate").get_to(_comp.uvTranslate);
    _j.at("color").get_to(_comp.color);
}
