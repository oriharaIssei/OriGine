#include "EmitterShape.h"

#include "myRandom/MyRandom.h"

/// math
#include "Matrix4x4.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

using namespace OriGine;

#ifdef _DEBUG
void EmitterShape::Debug([[maybe_unused]] const std::string& _parentLabel) {
    ImGui::Text("SpawnType : %s", kParticleSpawnLocationTypeWord[int(spawnType)].c_str());
    std::string label = kParticleSpawnLocationTypeWord[int(ParticleSpawnLocationType::InBody)] + "##" + _parentLabel;
    if (ImGui::RadioButton(label.c_str(), spawnType == ParticleSpawnLocationType::InBody)) {
        spawnType = ParticleSpawnLocationType::InBody;
    }
    label = kParticleSpawnLocationTypeWord[int(ParticleSpawnLocationType::Edge)] + "##" + _parentLabel;
    if (ImGui::RadioButton(label.c_str(), spawnType == ParticleSpawnLocationType::Edge)) {
        spawnType = ParticleSpawnLocationType::Edge;
    }
}
#endif // _DEBUG

#pragma region "Sphere"
#ifdef _DEBUG
void EmitterSphere::Debug([[maybe_unused]] const std::string& _parentLabel) {
    EmitterShape::Debug(_parentLabel);
    ImGui::Text("radius");
    std::string label = "##_radius" + _parentLabel;
    ImGui::DragFloat(label.c_str(), &radius_, 0.01f, 0.01f);
}
#endif // _DEBUG

Vec3f EmitterSphere::GetSpawnPos() {
    if (spawnType == ParticleSpawnLocationType::InBody) {
        // 球の内部(体積)から一様に近い分布でサンプリングする:
        // ランダムな方向ベクトルを正規化して単位球面上の点にし、
        // 別途 [0, radius_] でサンプリングした距離を掛けることで球内部の点にする
        MyRandom::Float randFloat = MyRandom::Float(0.0f, radius_);
        float randDist            = randFloat.Get();
        randFloat.SetRange(-1.0f, 1.0f);

        Vec3f randDire = {randFloat.Get(), randFloat.Get(), randFloat.Get()};
        randDire       = randDire.normalize();

        return randDire * randDist;
    } else { //==============Edge==============//
        // 球面(Edge)から一様分布でサンプリングする:
        // 球面座標系の偏角 theta(経度: 0〜2π)・phi(緯度方向: 0〜π)を一様乱数から求め、
        // 球面座標→直交座標に変換して半径 radius_ 倍する
        MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);
        float randTheta           = randFloat.Get() * 2.0f * 3.14159265358979323846f;
        randFloat.SetRange(-1.0f, 1.0f);
        float randPhi = randFloat.Get() * 3.14159265358979323846f;

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

    // GUI で min/max を独立に編集できるため、逆転(min > max)した場合に備えて
    // 各軸ごとに小さい方を min_、大きい方を max_ へ入れ直す
    min_ = MinElement(max_, min_);
    max_ = MaxElement(max_, min_);
}
#endif // _DEBUG

Vec3f EmitterBox::GetSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);
    float randX               = randFloat.Get();
    float randY               = randFloat.Get();
    float randZ               = randFloat.Get();

    Vec3f diff = Vec3f(max_) - Vec3f(min_);
    if (spawnType == ParticleSpawnLocationType::Edge) {
        // 各軸の乱数を 0 か 1 に丸めることで、min_/max_ で作られる直方体の
        // 「面」上(各軸のどちらかの端)に点を落とす。内部には入り込まない
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

    // ボックスは min_/max_ で軸並行(AABB)に定義されているため、
    // 最後に rotate_ 分の回転を適用してエミッターの向きに合わせる
    spawnPos = TransformVector(spawnPos, MakeMatrix4x4::RotateXYZ(rotate_));

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

Vec3f EmitterCapsule::GetSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    // direction_ 軸周りの断面円上(またはその内部)のオフセット方向
    Vec3f randDire = {randFloat.Get(), randFloat.Get(), randFloat.Get()};
    randDire       = randDire.normalize();

    // InBody は半径内のどこでも、Edge は円周(半径ちょうど)に固定する
    float randRadius = 0.0f;
    if (spawnType == ParticleSpawnLocationType::InBody) {
        randRadius = randFloat.Get() * radius_;
    } else { //==============Edge==============//
        randRadius = radius_;
    }

    // direction_ 軸方向に沿った位置は [0, length_] の範囲で一様にサンプリングする
    randFloat.SetRange(0.0f, length_);
    float randDist = randFloat.Get();

    // 軸方向の移動量 + 断面方向のオフセットで、カプセル(円柱)内の点を組み立てる
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

Vec3f EmitterCone::GetSpawnPos() {
    MyRandom::Float randFloat = MyRandom::Float(0.0f, 1.0f);

    Vec3f randDire = {randFloat.Get(), randFloat.Get(), randFloat.Get()};
    randDire       = randDire.normalize();

    // tan(angle_ / 2) は、軸方向に単位距離だけ進んだときの円錐断面半径に相当する値。
    // ただしここでは randDist(軸方向の距離)を掛けていないため、実際には
    // 軸からの距離によらず半径が一定の円柱状の分布になっている
    float randRadius = 0.0f;
    if (spawnType == ParticleSpawnLocationType::InBody) {
        randRadius = randFloat.Get() * std::tan(angle_ * 0.5f);
    } else { //==============Edge==============//
        randRadius = std::tan(angle_ * 0.5f);
    }

    randFloat.SetRange(0.0f, length_);
    float randDist = randFloat.Get();

    return (Vec3f(direction_) * randDist) + (randDire * randRadius);
}
#pragma endregion
