#pragma once

/// stl
#include <vector>

/// default data
#include "base/ReplayData.h"

/// util
#include "StringUtil.h"

namespace OriGine {
/// engine
class KeyboardInput;
class MouseInput;
class GamepadInput;

/// <summary>
/// ゲームプレイ中の入力情報をフレーム単位で記録し、ファイルに保存するクラス.
/// キーボード、マウス、ゲームパッドの各入力を BitArray 等を用いてバイナリ形式で蓄積する.
/// </summary>
class ReplayRecorder {
public:
    ReplayRecorder();
    ~ReplayRecorder();

    /// <summary>
    /// 記録の初期化処理.
    /// 以前の記録データをクリアし、開始時のシーン名を保持する.
    /// </summary>
    /// <param name="_startSceneName">リプレイ開始時のシーン名</param>
    void Initialize(const std::string& _startSceneName);

    /// <summary> 解放処理. </summary>
    void Finalize();

    /// <summary>
    /// 現在の 1 フレーム分の入力情報をキャプチャして内部リストに追加する.
    /// </summary>
    /// <param name="deltaTime">フレームの経過時間</param>
    /// <param name="_keyInput">キーボード入力オブジェクト（nullptr 可）</param>
    /// <param name="_mouseInput">マウス入力オブジェクト（nullptr 可）</param>
    /// <param name="_padInput">ゲームパッド入力オブジェクト（nullptr 可）</param>
    void RecordFrame(float deltaTime, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput);

    /// <summary>
    /// 蓄積されたリプレイデータをバイナリファイル（.rpd）として保存する.
    /// </summary>
    /// <param name="_directory">保存先のディレクトリパス</param>
    /// <param name="_filename">ファイル名（拡張子不要。デフォルトは現在時刻の文字列）</param>
    /// <returns>保存に成功したか</returns>
    bool SaveToFile(const std::string& _directory, const std::string& _filename = TimeToString());

private:
    /// <summary> ヘッダ情報（シーン名、バージョン、フレーム数）をファイルに書き込む. </summary>
    void WriteHeader(std::ofstream& _ofs);
    /// <summary> 指定したインデックスのフレームデータをファイルに書き込む. </summary>
    void WriteFrameData(std::ofstream& _ofs, size_t _frameIndex);

private:
    ReplayFileHeader header_             = {}; // 記録中のヘッダ情報
    std::vector<ReplayFrameData> frames_ = {}; // キャプチャされたフレームデータのリスト
};

} // namespace OriGine
