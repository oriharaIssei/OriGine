#include "ECS/component/text/TextComponent.h"

#include "imgui/imgui.h"
#include "text/FontManager.h"

namespace OriGine {

void TextComponent::Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {
	dirty = true;
}

void TextComponent::Finalize() {}

void TextComponent::Edit(Scene* /*_scene*/, const EntityHandle& /*_owner*/, const std::string& _parentLabel) {
	std::string label = _parentLabel + "##TextComponent";

	char buf[256] = {};
	std::strncpy(buf, text.c_str(), sizeof(buf) - 1);
	if (ImGui::InputText(("Text" + label).c_str(), buf, sizeof(buf))) {
		text = buf;
		dirty = true;
	}

	// フォント選択（FontManager に登録されたフォント一覧から選ぶ）
	{
		FontManager* fontManager = FontManager::GetInstance();
		const auto& fontList = fontManager->GetFontList();

		FontHandle effective = fontHandle;
		if (effective == kInvalidFontHandle) {
			effective = fontManager->GetDefault();
		}

		int currentIdx = 0;
		std::vector<const char*> names;
		names.reserve(fontList.size());
		for (size_t fi = 0; fi < fontList.size(); ++fi) {
			names.push_back(fontList[fi].name.c_str());
			if (fontList[fi].handle == effective) {
				currentIdx = static_cast<int>(fi);
			}
		}

		if (!names.empty()) {
			if (ImGui::Combo(("Font" + label).c_str(), &currentIdx, names.data(), static_cast<int>(names.size()))) {
				fontHandle = fontList[currentIdx].handle;
				dirty = true;
			}
		}
	}

	if (ImGui::DragFloat2(("Position" + label).c_str(), &position[X], 1.0f)) {
		dirty = true;
	}
	if (ImGui::DragFloat(("FontSize" + label).c_str(), &fontSize, 0.5f, 8.0f, 128.0f)) {
		dirty = true;
	}
	if (ImGui::DragFloat(("MaxWidth" + label).c_str(), &maxWidth, 1.0f, 0.0f, 4096.0f, "%.0f")) {
		dirty = true;
	}
	if (ImGui::DragFloat(("LineSpacing" + label).c_str(), &lineSpacing, 0.05f, 0.5f, 3.0f)) {
		dirty = true;
	}
	if (ImGui::DragFloat(("CharSpacing" + label).c_str(), &charSpacing, 0.1f, -20.0f, 100.0f, "%.1f")) {
		dirty = true;
	}

	const char* alignNames[] = {"Left", "Center", "Right"};
	int alignIdx = static_cast<int>(align);
	if (ImGui::Combo(("Align" + label).c_str(), &alignIdx, alignNames, 3)) {
		align = static_cast<TextAlign>(alignIdx);
		dirty = true;
	}

	ImGui::ColorEdit4(("Color" + label).c_str(), &color[X]);
	ImGui::DragInt(("Priority" + label).c_str(), &renderPriority);
	ImGui::Checkbox(("Visible" + label).c_str(), &visible);
	ImGui::Checkbox(("ShowBounds" + label).c_str(), &showBounds);
}

void to_json(nlohmann::json& j,const TextComponent& c){
	j["text"] = c.text;
	j["position"] = {c.position[X],c.position[Y]};
	j["color"] = {c.color[X],c.color[Y],c.color[Z],c.color[W]};
	j["fontHandle"] = c.fontHandle;
	j["fontSize"] = c.fontSize;
	j["maxWidth"] = c.maxWidth;
	j["lineSpacing"] = c.lineSpacing;
	j["charSpacing"] = c.charSpacing;
	j["align"] = static_cast<int>(c.align);
	j["renderPriority"] = c.renderPriority;
	j["visible"] = c.visible;
	j["showBounds"] = c.showBounds;
}

void from_json(const nlohmann::json& j,TextComponent& c){
	j["text"].get_to(c.text);
	j["position"].get_to(c.position);
	j["color"].get_to(c.color);
	j["fontHandle"].get_to(c.fontHandle);
	if (j.contains("fontSize")) j["fontSize"].get_to(c.fontSize);
	if (j.contains("maxWidth")) j["maxWidth"].get_to(c.maxWidth);
	if (j.contains("lineSpacing")) j["lineSpacing"].get_to(c.lineSpacing);
	if (j.contains("charSpacing")) j["charSpacing"].get_to(c.charSpacing);
	if (j.contains("align")) { int a = 0; j["align"].get_to(a); c.align = static_cast<TextAlign>(a); }
	j["renderPriority"].get_to(c.renderPriority);
	j["visible"].get_to(c.visible);
	if (j.contains("showBounds")) j["showBounds"].get_to(c.showBounds);
}

} // namespace OriGine
