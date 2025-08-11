#include "EmitterShape.h"

/// lib
#include "binaryIO/BinaryIO.h"
#include "myRandom/MyRandom.h"

/// math
#include "Matrix4x4.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#ifdef _DEBUG
void EmitterShape::Debug([[maybe_unused]] const std::string& _parentLabel) {
    ImGui::Text("SpawnType : %s", particleSpawnLocationTypeWord_[int(spawnType_)].c_str());
    std::string label = particleSpawnLocationTypeWord_[int(ParticleSpawnLocationType::InBody)] + "##" + _parentLabel;
    if (ImGui::RadioButton(label.c_str(), spawnType_ == ParticleSpawnLocationType::InBody)) {
        spawnType_ = ParticleSpawnLocationType::InBody;
    }
    label = particleSpawnLocationTypeWord_[int(ParticleSpawnLocationType::Edge)] + "##" + _parentLabel;
    if (ImGui::RadioButton(label.c_str(), spawnType_ == ParticleSpawnLocationType::Edge)) {
        spawnType_ = ParticleSpawnLocationType::Edge;
    }
}
#endif // _DEBUG

#pragma region "Sphere"
#ifdef _DEBUG
void EmitterSphere::Debug([[maybe_unused]] const std::string& _parentLabel) {
    EmitterShape::Debug(_parentLabel);
    ImGui::Text("radius");
    std::string label = "##_radius" +_parentLabel;
    ImGui::DragFloat(label.c_str(), &radius_, 0.01f, 0.01f);
}
#endif // _DEBUG

Vec3f EmitterSphere::getSpawnPos() {
    if (spawnType_ == ParticleSpawnLocationType::InBody) {
        MyRandom::Float randFloat = MyRandom::Float(0.0f, radius_);
        float randDist            = randFloat.get();
        randFloat.setRange(-1.0f, 1.0f);

        Vec3f randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
        randDire       = randDire.normalize();

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

#pragma region "Box"
#ifdef _DEBUG
void EmitterBox::Debug([[maybe_unused]] const std::string& _parentLabel) {
    EmitterShape::Debug(_parentLabel);
    ImGui::Text("min");
    std::string label = "##" + _parentLabel + "_min";
    ImGui::DragFloat3(label.c_str(), min_.v, 0.1f);

    ImGui::Text("min");
    label = "##" + _parentLabel + "_max";
    ImGui::DragFloat3(label.c_str(), max_.v, 0.1f);

    min_ = MinElement(max_, min_);
    max_ = MaxElement(max_, min_);
}
#endif // _DEBUG

Vec3f EmitterBox::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);
    float randX               = randFloat.get();
    float randY               = randFloat.get();
    float randZ               = randFloat.get();

    Vec3f diff = Vec3f(max_) - Vec3f(min_);
    if (spawnType_ == ParticleSpawnLocationType::Edge) {
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
        min_[X] + diff[X] * randX,
        min_[Y] + diff[Y] * randY,
        min_[Z] + diff[Z] * randZ);

    spawnPos = TransformVector(spawnPos, MakeMatrix::RotateXYZ(rotate_));

    return spawnPos;
}

#pragma endregion

#pragma region "Capsule"
#ifdef _DEBUG
void EmitterCapsule::Debug([[maybe_unused]] const std::string& _parentLabel) {
    EmitterShape::Debug(_parentLabel);
    ImGui::Text("direction");
    std::string label = "##" + _parentLabel + "_direction";
    if (ImGui::DragFloat3(label.c_str(), direction_.v, 0.1f)) {
        direction_ = (direction_.normalize());
    }

    ImGui::Text("radius");
    label = "##" + _parentLabel + "_radius";
    ImGui::DragFloat(label.c_str(), &radius_, 0.1f);

    ImGui::Text("length");
    label = "##" + _parentLabel + "_length";
    ImGui::DragFloat(label.c_str(), &length_, 0.1f);
}

#endif // _DEBUG

Vec3f EmitterCapsule::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    Vec3f randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
    randDire       = randDire.normalize();

    float randRadius = 0.0f;
    if (spawnType_ == ParticleSpawnLocationType::InBody) {
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
void EmitterCone::Debug([[maybe_unused]] const std::string& _parentLabel) {
    EmitterShape::Debug(_parentLabel);
    ImGui::Text("direction");
    std::string label = "##" + _parentLabel + "_direction";
    if (ImGui::DragFloat3(label.c_str(), direction_.v, 0.1f)) {
        direction_ = (direction_.normalize());
    }

    ImGui::Text("angle");
    label = "##" + _parentLabel + "_angle";
    ImGui::DragFloat(label.c_str(), &angle_, 0.1f);

    ImGui::Text("length");
    label = "##" + _parentLabel + "_length";
    ImGui::DragFloat(label.c_str(), &length_, 0.1f);
}
#endif // _DEBUG

Vec3f EmitterCone::getSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    Vec3f randDire = {randFloat.get(), randFloat.get(), randFloat.get()};
    randDire       = randDire.normalize();

    float randRadius = 0.0f;
    if (spawnType_ == ParticleSpawnLocationType::InBody) {
        randRadius = randFloat.get() * std::tan(angle_ * 0.5f);
    } else { //==============Edge==============//
        randRadius = std::tan(angle_ * 0.5f);
    }

    randFloat.setRange(0.0f, length_);
    float randDist = randFloat.get();

    return (Vec3f(direction_) * randDist) + (randDire * randRadius);
}
#pragma endregion
