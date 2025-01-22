#include "EmitterShape.h"

//lib
#include "myRandom/MyRandom.h"

//math
#include "Matrix4x4.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#ifdef _DEBUG
void EmitterShape::Debug() {
    ImGui::Text("SpawnType : %s", particleSpawnLocationTypeWord_[int(spawnType_)].c_str());
    if (ImGui::RadioButton(particleSpawnLocationTypeWord_[int(ParticleSpawnLocationType::InBody)].c_str(), spawnType_ == int32_t(ParticleSpawnLocationType::InBody))) {
        spawnType_.setValue(int32_t(ParticleSpawnLocationType::InBody));
    }
    if (ImGui::RadioButton(particleSpawnLocationTypeWord_[int(ParticleSpawnLocationType::Edge)].c_str(), spawnType_ == int32_t(ParticleSpawnLocationType::Edge))) {
        spawnType_.setValue(int32_t(ParticleSpawnLocationType::Edge));
    }
}
#endif // _DEBUG

#pragma region "Sphere"
#ifdef _DEBUG
void EmitterSphere::Debug() {
    EmitterShape::Debug();
    ImGui::Text("radius");
    std::string label = "##" + emitterShapeTypeWord_[int(type_)] + "_radius";
    ImGui::DragFloat(label.c_str(), radius_, 0.1f);
}
#endif // _DEBUG

Vec3f EmitterSphere::getSpawnPos() {
    if (spawnType_ == int32_t(ParticleSpawnLocationType::InBody)) {
        MyRandom::Float randFloat = MyRandom::Float(0.0f, radius_);
        float randDist            = randFloat.get();
        randFloat.setRange(-1.0f, 1.0f);

        Vec3f randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
        randDire         = randDire.normalize();

        return randDire * randDist;
    } else { //==============Edge==============//
        MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);
        float randTheta           = randFloat.get() * 2.0f * 3.14159265358979323846f;
        randFloat.setRange(-1.0f, 1.0f);
        float randPhi = randFloat.get() * 3.14159265358979323846f;

        Vec3f randDire = {std::cos(randTheta) * std::sin(randPhi), std::cos(randPhi), std::sin(randTheta) * std::sin(randPhi)};

        return randDire * radius_;
    }
}
#pragma endregion

#pragma region "Obb"
#ifdef _DEBUG
void EmitterOBB::Debug() {
    EmitterShape::Debug();
    ImGui::Text("min");
    std::string label = "##" + emitterShapeTypeWord_[int(type_)] + "_min";
    ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(min_.operator Vec3f*()), 0.1f);

    ImGui::Text("min");
    label = "##" + emitterShapeTypeWord_[int(type_)] + "_max";
    ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(max_.operator Vec3f*()), 0.1f);

    min_.setValue({(std::min)(min_->v[X], max_->v[X]), (std::min)(min_->v[Y], max_->v[Y]), (std::min)(min_->v[Z], max_->v[Z])});
    max_.setValue({(std::max)(min_->v[X], max_->v[X]), (std::max)(min_->v[Y], max_->v[Y]), (std::max)(min_->v[Z], max_->v[Z])});
}
#endif // _DEBUG

Vec3f EmitterOBB::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);
    float randX               = randFloat.get();
    float randY               = randFloat.get();
    float randZ               = randFloat.get();

    Vec3f diff = Vec3f(max_) - Vec3f(min_);
    if (spawnType_ == int32_t(ParticleSpawnLocationType::Edge)) {
        if (randX < 0.5f) {
            randX = 0.0f;
        } else {
            randX = 1.0f;
        }
        if (randY < 0.5f) {
            randY = 0.0f;
        } else {
            randY = 1.0f;
        }
        if (randZ < 0.5f) {
            randZ = 0.0f;
        } else {
            randZ = 1.0f;
        }
    }
    Vec3f spawnPos = Vec3f(
        min_->v[X] + diff[X] * randX,
        min_->v[Y] + diff[Y] * randY,
        min_->v[Z] + diff[Z] * randZ);

    spawnPos = TransformVector(spawnPos, MakeMatrix::RotateXYZ(rotate_));

    return spawnPos;
}

#pragma endregion

#pragma region "Capsule"
#ifdef _DEBUG
void EmitterCapsule::Debug() {
    EmitterShape::Debug();
    ImGui::Text("direction");
    std::string label = "##" + emitterShapeTypeWord_[int(type_)] + "_direction";
    if (ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(direction_.operator Vec3f*()), 0.1f)) {
        direction_.setValue(direction_->normalize());
    }

    ImGui::Text("radius");
    label = "##" + emitterShapeTypeWord_[int(type_)] + "_radius";
    ImGui::DragFloat(label.c_str(), radius_, 0.1f);

    ImGui::Text("length");
    label = "##" + emitterShapeTypeWord_[int(type_)] + "_length";
    ImGui::DragFloat(label.c_str(), length_, 0.1f);
}

#endif // _DEBUG

Vec3f EmitterCapsule::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    Vec3f randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
    randDire         = randDire.normalize();

    float randRadius = 0.0f;
    if (spawnType_ == int32_t(ParticleSpawnLocationType::InBody)) {
        randRadius = randFloat.get() * radius_;
    } else { //==============Edge==============//
        randRadius = radius_;
    }

    randFloat.setRange(0.0f, length_);
    float randDist = randFloat.get();

    return (Vec3f(direction_) * randDist) + (randDire * randRadius);
}
#pragma endregion

#pragma region "Cone"
#ifdef _DEBUG
void EmitterCone::Debug() {
    EmitterShape::Debug();
    ImGui::Text("direction");
    std::string label = "##" + emitterShapeTypeWord_[int(type_)] + "_direction";
    if (ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(direction_.operator Vec3f*()), 0.1f)) {
        direction_.setValue(direction_->normalize());
    }

    ImGui::Text("angle");
    label = "##" + emitterShapeTypeWord_[int(type_)] + "_angle";
    ImGui::DragFloat(label.c_str(), angle_, 0.1f);

    ImGui::Text("length");
    label = "##" + emitterShapeTypeWord_[int(type_)] + "_length";
    ImGui::DragFloat(label.c_str(), length_, 0.1f);
}
#endif // _DEBUG

Vec3f EmitterCone::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    Vec3f randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
    randDire         = randDire.normalize();

    float randRadius = 0.0f;
    if (spawnType_ == int32_t(ParticleSpawnLocationType::InBody)) {
        randRadius = randFloat.get() * std::tan(angle_ * 0.5f);
    } else { //==============Edge==============//
        randRadius = std::tan(angle_ * 0.5f);
    }

    randFloat.setRange(0.0f, length_);
    float randDist = randFloat.get();

    return (Vec3f(direction_) * randDist) + (randDire * randRadius);
}
#pragma endregion
