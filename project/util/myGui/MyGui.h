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
/// <param name="_label"></param>
/// <param name="_value"></param>
/// <returns></returns>
bool CheckBoxCommand(const std::string& _label, bool& _value);

/// <summary>
/// ::ImGui::Button をboolに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
/// <param name="_label"></param>
/// <param name="_value"></param>
/// <returns></returns>
bool ButtonCommand(const std::string& _label, bool& _value);

/// <summary>
/// マウス座標をシーンビューの座標に変換する
/// </summary>
/// <param name="_mousePos"></param>
/// <param name="_sceneViewPos"></param>
/// <param name="_sceneViewSize"></param>
/// <param name="_originalResolution"></param>
/// <returns></returns>
OriGine::Vec2f ConvertMouseToSceneView(const OriGine::Vec2f& _mousePos, const ImVec2& _sceneViewPos, const ImVec2& _sceneViewSize, const OriGine::Vec2f& _originalResolution);

/// <summary>
/// テクスチャ読み込みボタンを表示する
/// </summary>
/// <param name="_texIndex"></param>
/// <param name="_parentLabel"></param>
/// <returns></returns>
bool AskLoadTextureButton(size_t _texIndex, const std::string& _parentLabel);

/// <summary>
/// dialogを開いてファイルを選択させる(directoryとfilenameを分ける)
/// </summary>
/// <param name="_baseDirectory"></param>
/// <param name="_outputDirectory"></param>
/// <param name="_outputFileName"></param>
/// <param name="_extension">対象のファイル拡張子</param>
/// <param name="_withoutExtension">出力に拡張子を含めるかどうか. true = 含めない,false = 含める</param>
/// <param name="_withoutBaseDirectory">出力に_baseDirectoryを含めるかどうか. true ＝ 含めない, false = 含める</param>
/// <returns>選択に成功したかどうか. true = 成功した, false = 失敗した</returns>
bool OpenFileDialog(const std::string& _baseDirectory, std::string& _outputDirectory, std::string& _outputFileName, const std::vector<std::string>& _extension, bool _withoutExtension, bool _withoutBaseDirectory);

/// <summary>
/// dialogを開いてファイルを選択させる(directoryとfilenameを合わせる)
/// </summary>
/// <param name="_baseDirectory"></param>
/// <param name="_outputPath">出力</param>
/// <param name="_extension">対象のファイル拡張子</param>
/// <param name="_withoutExtension">出力に拡張子を含めるかどうか. true = 含めない,false = 含める</param>
/// <param name="_withoutBaseDirectory">出力に_baseDirectoryを含めるかどうか. true ＝ 含めない, false = 含める</param>
/// <returns>選択に成功したかどうか. true = 成功した, false = 失敗した</returns>
bool OpenFileDialog(const std::string& _baseDirectory, std::string& _outputPath, const std::vector<std::string>& _extension, bool _withoutExtension = false, bool _withoutBaseDirectory = false);

#endif // _DEBUG
