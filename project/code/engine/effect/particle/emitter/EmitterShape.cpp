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

Vector3 EmitterSphere::getSpawnPos() {
    if (spawnType_ == int32_t(ParticleSpawnLocationType::InBody)) {
        MyRandom::Float randFloat = MyRandom::Float(0.0f, radius_);
        float randDist            = randFloat.get();
        randFloat.setRange(-1.0f, 1.0f);

        Vector3 randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
        randDire         = randDire.normalize();

        return randDire * randDist;
    } else { //==============Edge==============//
        MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);
        float randTheta           = randFloat.get() * 2.0f * 3.14159265358979323846f;
        randFloat.setRange(-1.0f, 1.0f);
        float randPhi = randFloat.get() * 3.14159265358979323846f;

        Vector3 randDire = {std::cos(randTheta) * std::sin(randPhi), std::cos(randPhi), std::sin(randTheta) * std::sin(randPhi)};

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
    ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(min_.operator Vector3*()), 0.1f);

    ImGui::Text("min");
    label = "##" + emitterShapeTypeWord_[int(type_)] + "_max";
    ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(max_.operator Vector3*()), 0.1f);

    min_.setValue({(std::min)(min_->x, max_->x), (std::min)(min_->y, max_->y), (std::min)(min_->z, max_->z)});
    max_.setValue({(std::max)(min_->x, max_->x), (std::max)(min_->y, max_->y), (std::max)(min_->z, max_->z)});
}
#endif // _DEBUG

Vector3 EmitterOBB::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);
    float randX               = randFloat.get();
    float randY               = randFloat.get();
    float randZ               = randFloat.get();

    Vector3 diff = Vector3(max_) - Vector3(min_);
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
    Vector3 spawnPos = Vector3(
        min_->x + diff.x * randX,
        min_->y + diff.y * randY,
        min_->z + diff.z * randZ);

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
    if (ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(direction_.operator Vector3*()), 0.1f)) {
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

Vector3 EmitterCapsule::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    Vector3 randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
    randDire         = randDire.normalize();

    float randRadius = 0.0f;
    if (spawnType_ == int32_t(ParticleSpawnLocationType::InBody)) {
        randRadius = randFloat.get() * radius_;
    } else { //==============Edge==============//
        randRadius = radius_;
    }

    randFloat.setRange(0.0f, length_);
    float randDist = randFloat.get();

    return (Vector3(direction_) * randDist) + (randDire * randRadius);
}
#pragma endregion

#pragma region "Cone"
#ifdef _DEBUG
void EmitterCone::Debug() {
    EmitterShape::Debug();
    ImGui::Text("direction");
    std::string label = "##" + emitterShapeTypeWord_[int(type_)] + "_direction";
    if (ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(direction_.operator Vector3*()), 0.1f)) {
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

Vector3 EmitterCone::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    Vector3 randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
    randDire         = randDire.normalize();

    float randRadius = 0.0f;
    if (spawnType_ == int32_t(ParticleSpawnLocationType::InBody)) {
        randRadius = randFloat.get() * std::tan(angle_ * 0.5f);
    } else { //==============Edge==============//
        randRadius = std::tan(angle_ * 0.5f);
    }

    randFloat.setRange(0.0f, length_);
    float randDist = randFloat.get();

    return (Vector3(direction_) * randDist) + (randDire * randRadius);
}
#pragma endregion
