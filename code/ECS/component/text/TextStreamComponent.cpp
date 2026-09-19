#include "ECS/component/text/TextStreamComponent.h"

#include "imgui/imgui.h"

#include <limits>

namespace OriGine {

void TextStreamComponent::Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {
	Reset();
	textHash = 0;
}

void TextStreamComponent::Finalize() {}

void TextStreamComponent::Reset() {
	revealed     = 0.0f;
	elapsedDelay = 0.0f;
	finished     = false;
	lastApplied  = -1;
}

void TextStreamComponent::Skip() {
	// 十分大きな値にしておけば、システム側で全文字数にクランプされる
	revealed = (std::numeric_limits<float>::max)();
}

void TextStreamComponent::Edit(Scene* /*_scene*/, const EntityHandle& /*_owner*/, const std::string& _parentLabel) {
	std::string label = _parentLabel + "##TextStreamComponent";

	ImGui::TextUnformatted("Text is owned by TextComponent.text");

	ImGui::DragFloat(("CharsPerSec" + label).c_str(), &charsPerSecond, 1.0f, 1.0f, 240.0f, "%.0f");
	ImGui::DragFloat(("StartDelay" + label).c_str(), &startDelay, 0.05f, 0.0f, 10.0f, "%.2f");

	ImGui::Checkbox(("Loop" + label).c_str(), &loop);
	ImGui::SameLine();
	ImGui::Checkbox(("Playing" + label).c_str(), &playing);

	if (ImGui::Button(("Play" + label).c_str())) Play();
	ImGui::SameLine();
	if (ImGui::Button(("Pause" + label).c_str())) Pause();
	ImGui::SameLine();
	if (ImGui::Button(("Reset" + label).c_str())) Reset();
	ImGui::SameLine();
	if (ImGui::Button(("Skip" + label).c_str())) Skip();

	ImGui::Text("Visible: %d", lastApplied < 0 ? 0 : lastApplied);
}

} // namespace OriGine
