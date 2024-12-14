#include "AnimationEditor.h"

#include "model/Model.h"

#include <algorithm>

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
namespace ImGui{
	bool TimeLineButtons(const std::string& _label,
						 std::vector<float>& _times,
						 float _min,
						 float _max){
		// ラベルを表示
		Text(_label.c_str());
		SameLine();

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		// スライダーの背景を描画
		ImDrawList* draw_list = GetWindowDrawList();

		const float sliderHeight = 20.0f; // スライダーの高さ
		const ImVec2 cursorPos = GetCursorScreenPos();
		const ImVec2 sliderSize(CalcItemWidth(),sliderHeight);
		const ImU32 sliderBgColor = IM_COL32(100,100,100,255); // スライダー背景色

		// 背景矩形を描画
		ImVec2 rectEnd(cursorPos.x + sliderSize.x,cursorPos.y + sliderSize.y);
		draw_list->AddRectFilled(cursorPos,rectEnd,sliderBgColor,style.FrameRounding);

		// スライダー背景の領域を保持
		ImRect sliderRect(cursorPos,rectEnd);

		// スライダーの入力領域を登録
		const ImGuiID id = GetID(_label.c_str());
		ItemSize(sliderSize,GetStyle().FramePadding.y);
		if(!ItemAdd(sliderRect,id))
			return false;

		// ドラッグ中の情報を保持するための変数
		static int draggedIndex = -1; // 現在ドラッグ中のボタンのインデックス
		static float draggedValue = 0.0f; // ドラッグ中の値

		const float buttonSize = 10.0f; // ボタンの幅（ピクセル）

		// 各ボタンを描画
		for(int i = 0; i < _times.size(); ++i){
			float t = (_times[i] - _min) / (_max - _min); // 正規化された位置
			float buttonX = sliderRect.Min.x + t * sliderSize.x;
			ImVec2 buttonPos(buttonX - buttonSize * 0.5f,sliderRect.Min.y);
			ImVec2 buttonEnd(buttonPos.x + buttonSize,buttonPos.y + sliderHeight);

			// ボタンの位置を取得
			ImRect buttonRect(buttonPos,buttonEnd);
			bool isHovered = IsMouseHoveringRect(buttonRect.Min,buttonRect.Max);

			// ドラッグ処理 ( すでに Drag しているときは 新しい ボタンを 動かさない )
			if(isHovered && IsMouseClicked(0) && draggedValue != -1){
				draggedIndex = i; // ドラッグを開始
				draggedValue = _times[i];
				SetActiveID(id,GetCurrentWindow()); // ボタンをアクティブに設定
				FocusWindow(GetCurrentWindow());
			}

			bool isActive = (draggedIndex == i);
			if(isActive && IsMouseDragging(0)){
				// マウス位置から新しい位置を計算
				float newT = (GetMousePos().x - sliderRect.Min.x) / sliderSize.x;
				newT = ImClamp(newT,0.0f,1.0f); // スライダー範囲内にクランプ
				draggedValue = _min + newT * (_max - _min); // 新しい値
				_times[i] = draggedValue; // 値を更新
			}

			if(isActive && IsMouseReleased(0)){
				draggedIndex = -1; // ドラッグを終了
				// `_times` を位置に基づいてソート
				std::sort(_times.begin(),_times.end());
			}

			// ボタンを描画
			PushID(i);
			draw_list->AddRectFilled(buttonRect.Min,buttonRect.Max,IM_COL32(200,200,200,255),style.FrameRounding);

			// アイテムオーバーラップを許可（ウィンドウ外部との干渉防止）
			SetItemAllowOverlap();
			PopID();
		}
		return true;
	}
}
#endif // _DEBUG

AnimationEditor::AnimationEditor(){}

AnimationEditor::~AnimationEditor(){}

void AnimationEditor::Init(){}

void AnimationEditor::Update(){}
