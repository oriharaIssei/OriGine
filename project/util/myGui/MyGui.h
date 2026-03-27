/**
 * @file MyGui.h
 * @brief ImGuiラッパーのメインヘッダー
 *
 * 基本GUIラッパーはMyGuiBase.hに、
 * コマンド対応版はMyGuiCommand.hに分離されています。
 */
#pragma once

#ifdef _DEBUG

/// 分離したテンプレート実装をinclude
#include "MyGuiBase.h"
#include "MyGuiCommand.h"

/// math
#include "math/Vector.h"

/// <summary>
/// ::ImGui::CheckBox をboolに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <param name="label"></param>
/// <param name="value"></param>
/// <returns></returns>
bool CheckBoxCommand(const std::string& label, bool& value);

/// <summary>
/// ::ImGui::Button をboolに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <param name="label"></param>
/// <param name="value"></param>
/// <returns></returns>
bool ButtonCommand(const std::string& label, bool& value);

/// <summary>
/// マウス座標をシーンビューの座標に変換する
/// </summary>
/// <param name="mousePos"></param>
/// <param name="sceneViewPos"></param>
/// <param name="sceneViewSize"></param>
/// <param name="originalResolution"></param>
/// <returns></returns>
OriGine::Vec2f ConvertMouseToSceneView(const OriGine::Vec2f& mousePos, const ImVec2& sceneViewPos, const ImVec2& sceneViewSize, const OriGine::Vec2f& originalResolution);

/// <summary>
/// テクスチャ読み込みボタンを表示する
/// </summary>
/// <param name="texIndex"></param>
/// <param name="parentLabel"></param>
/// <returns></returns>
bool AskLoadTextureButton(size_t texIndex, const std::string& parentLabel);

/// <summary>
/// dialogを開いてファイルを選択させる(directoryとfilenameを分ける)
/// </summary>
/// <param name="baseDirectory"></param>
/// <param name="outputDirectory"></param>
/// <param name="outputFileName"></param>
/// <param name="extension">対象のファイル拡張子</param>
/// <param name="withoutExtension">出力に拡張子を含めるかどうか. true = 含めない,false = 含める</param>
/// <param name="withoutBaseDirectory">出力に_baseDirectoryを含めるかどうか. true ＝ 含めない, false = 含める</param>
/// <returns>選択に成功したかどうか. true = 成功した, false = 失敗した</returns>
bool OpenFileDialog(const std::string& baseDirectory, std::string& outputDirectory, std::string& outputFileName, const std::vector<std::string>& extension, bool withoutExtension, bool withoutBaseDirectory);

/// <summary>
/// dialogを開いてファイルを選択させる(directoryとfilenameを合わせる)
/// </summary>
/// <param name="baseDirectory"></param>
/// <param name="outputPath">出力</param>
/// <param name="extension">対象のファイル拡張子</param>
/// <param name="withoutExtension">出力に拡張子を含めるかどうか. true = 含めない,false = 含める</param>
/// <param name="withoutBaseDirectory">出力に_baseDirectoryを含めるかどうか. true ＝ 含めない, false = 含める</param>
/// <returns>選択に成功したかどうか. true = 成功した, false = 失敗した</returns>
bool OpenFileDialog(const std::string& baseDirectory, std::string& outputPath, const std::vector<std::string>& extension, bool withoutExtension = false, bool withoutBaseDirectory = false);

#endif // _DEBUG
