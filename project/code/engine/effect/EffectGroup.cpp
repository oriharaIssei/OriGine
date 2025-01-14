#include "EffectGroup.h"

///engine
//Object
#include "object3d/AnimationObject3d.h"
#include "particle/emitter/Emitter.h"

EffectGroup::EffectGroup(const std::string& _name)
    : name_(_name) {}

EffectGroup::~EffectGroup() {}

void EffectGroup::Init() {
    LoadEffectGroup();
}

void EffectGroup::Update() {
}

#pragma region "IO"
void EffectGroup::LoadEffectGroup() {
}

void EffectGroup::SaveEffectGroup() {
}
#pragma endregion
