#include "ParticleTransform.h"

void ParticleTransform::UpdateMatrix(){
	worldMat = MakeMatrix4x4::Affine(scale,rotate,translate);
	uvMat    = MakeMatrix4x4::Affine(uvScale,uvRotate,uvTranslate);

	if(parentWorldMat){
        worldMat *= *parentWorldMat;
	}
}

void to_json(nlohmann::json& j, const ParticleTransform& r) {
    j = nlohmann::json{
        {"scale", r.scale},
        {"rotate", r.rotate},
        {"translate", r.translate},
        {"uvScale", r.uvScale},
        {"uvRotate", r.uvRotate},
        {"uvTranslate", r.uvTranslate},
        {"color", r.color}};
}
void from_json(const nlohmann::json& j, ParticleTransform& r) {
    j.at("scale").get_to(r.scale);
    j.at("rotate").get_to(r.rotate);
    j.at("translate").get_to(r.translate);
    j.at("uvScale").get_to(r.uvScale);
    j.at("uvRotate").get_to(r.uvRotate);
    j.at("uvTranslate").get_to(r.uvTranslate);
    j.at("color").get_to(r.color);
}
