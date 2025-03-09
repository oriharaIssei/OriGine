#include "Shapes.h"

///lib
#include "BinaryIO/BinaryIO.h"

/// externals
#include "imgui/imgui.h"

#pragma region "Sphere"
bool Sphere::Edit() {
    bool isChange = false;
    ImGui::Text("Center");
    isChange |= ImGui::DragFloat3("##Center", center_.v, 0.01f);
    ImGui::Text("Radius");
    isChange |= ImGui::DragFloat("##Radius", &radius_, 0.01f);

    return isChange;
}

void Sphere::Save(BinaryWriter& _writer) {
    _writer.WriteVec(center_);
    _writer.Write(radius_);
}

void Sphere::Load(BinaryReader& _reader) {
    _reader.ReadVec(center_);
    _reader.Read(radius_);
}
#pragma endregion

#pragma region "AABB"

bool AABB::Edit() {
    bool isChange = false;
    ImGui::Text("Min");
    isChange |= ImGui::DragFloat3("##Min", min_.v, 0.01f);
    ImGui::Text("Max");
    isChange |= ImGui::DragFloat3("##Max", max_.v, 0.01f);

    return isChange;
}

void AABB::Save(BinaryWriter& _writer) {
    _writer.WriteVec(min_);
    _writer.WriteVec(max_);
}

void AABB::Load(BinaryReader& _reader) {
    _reader.ReadVec(min_);
    _reader.ReadVec(max_);
}

#pragma endregion

#pragma region "OBB"

bool OBB::Edit() {
    bool isChange = false;

    ImGui::Text("Min");
    isChange |= ImGui::DragFloat3("##Min", min_.v, 0.01f);
    ImGui::Text("Max");
    isChange |= ImGui::DragFloat3("##Max", max_.v, 0.01f);

    ImGui::Spacing();

    ImGui::Text("Rotate");
    if (ImGui::DragFloat4("##Rotate", rotate_.v, 0.01f)) {
        rotate_  = rotate_.normalize();
        isChange = true;
    }

    return isChange;
}

void OBB::Save(BinaryWriter& _writer) {
    _writer.WriteVec(min_);
    _writer.WriteVec(max_);
    _writer.WriteVec(rotate_);
}

void OBB::Load(BinaryReader& _reader) {
    _reader.ReadVec(min_);
    _reader.ReadVec(max_);
    _reader.ReadVec(rotate_);
}

#pragma endregion

#pragma region "Capsule"

bool Capsule::Edit() {
    bool isChange = false;

    ImGui::Text("Start");
    isChange |= ImGui::DragFloat3("##Start", start_.v, 0.01f);
    ImGui::Text("End");
    isChange |= ImGui::DragFloat3("##End", end_.v, 0.01f);

    ImGui::Spacing();

    ImGui::Text("Radius");
    isChange |= ImGui::DragFloat("##Radius", &radius_, 0.01f);

    return isChange;
}

void Capsule::Save(BinaryWriter& _writer) {
    _writer.WriteVec(start_);
    _writer.WriteVec(end_);
    _writer.Write(radius_);
}

void Capsule::Load(BinaryReader& _reader) {
    _reader.ReadVec(start_);
    _reader.ReadVec(end_);
    _reader.Read(radius_);
}

#pragma endregion
