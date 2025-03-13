#include "Rigidbody.h"

/// externals
#include "imgui/imgui.h"

Rigidbody::Rigidbody() {}

void Rigidbody::Init(GameEntity* _entity) {
    _entity;
}

bool Rigidbody::Edit() {
    bool isChange = false;

    isChange |= ImGui::InputFloat3("acceleration", acceleration_.v);
    isChange |= ImGui::InputFloat3("velocity", velocity_.v);

    ImGui::Separator();

    isChange |= ImGui::InputFloat("mass", &mass_);
    isChange |= ImGui::Checkbox("useGravity", &useGravity_);
    return isChange;
}

void Rigidbody::Save(BinaryWriter& _writer) {
    _writer.Write<3, float>(acceleration_);
    _writer.Write<3, float>(velocity_);
    _writer.Write(mass_);
    _writer.Write(useGravity_);
}

void Rigidbody::Load(BinaryReader& _reader) {
    _reader.Read<3, float>(acceleration_);
    _reader.Read<3, float>(velocity_);
    _reader.Read(mass_);
    _reader.Read(useGravity_);
}

void Rigidbody::Finalize() {}
