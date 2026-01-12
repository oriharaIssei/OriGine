#pragma once

/// stl
#include <string>
#include <vector>

/// default data
#include "base/ReplayData.h"

/// external
#include "logger/Logger.h"

namespace OriGine {
/// engine
class SceneManager;
// input
class KeyboardInput;
class MouseInput;
class GamepadInput;

/// <summary>
/// 記録されたリプレイデータ（.rpd）を読み込み、フレーム単位で入力を再現するクラス.
/// キーボード、マウス、ゲームパッドの状態を指定された入力デバイスに注入（Inject）する.
/// </summary>
class ReplayPlayer {
public:
    /// <summary>
    /// 指定されたファイルパスのリプレイデータを読み込み、初期化する.
    /// シーンマネージャーを介して、記録開始時のシーンへの遷移も行う.
    /// </summary>
    /// <param name="_path">リプレイファイルのパス</param>
    /// <param name="_sceneManager">シーン遷移を制御するマネージャーのポインタ</param>
    void Initialize(const std::string& _path, SceneManager* _sceneManager);

    /// <summary> 解放処理. インデックスのリセット等を行う. </summary>
    void Finalize();

    /// <summary>
    /// 指定したファイルから記録データをバイナリとして読み込む.
    /// </summary>
    /// <param name="_path">ファイルパス（ディレクトリ + ファイル名）</param>
    /// <returns>読み込みに成功したか</returns>
    bool LoadFromFile(const std::string& _path);

    /// <summary>
    /// 現在のフレームのリプレイ入力を指定された入力デバイスに適用する.
    /// 適用後、フレームインデックスを 1 進める.
    /// </summary>
    /// <param name="key">キーボード入力オブジェクト（上書きされる）</param>
    /// <param name="mouse">マウス入力オブジェクト（上書きされる）</param>
    /// <param name="pad">ゲームパッド入力オブジェクト（上書きされる）</param>
    /// <returns>記録時されたそのフレームの経過時間（deltaTime）</returns>
    float Apply(KeyboardInput* key, MouseInput* mouse, GamepadInput* pad);

    /// <summary>
    /// 再生位置（フレームインデックス）を任意の場所に移動させる.
    /// </summary>
    /// <param name="frameIndex">ジャンプ先のフレーム番号</param>
    /// <returns>シークに成功したか</returns>
    bool Seek(size_t frameIndex);

    /// <summary>
    /// 全フレームの再生が終了したか（CurrentIndex が総フレーム数以上になったか）.
    /// </summary>
    bool IsEnd() const { return currentFrameIndex_ >= fileData_.frameData.size(); }

    /// <summary>
    /// 指定したフレームインデックスが範囲内（有効）かどうかを判定する.
    /// </summary>
    /// <param name="frameIndex">判定対象のインデックス</param>
    /// <returns>有効であれば true</returns>
    bool IsValidFrame(int32_t frameIndex) const {
        return (frameIndex >= 0 && frameIndex < static_cast<int32_t>(fileData_.frameData.size()));
    }

private:
    ReplayFile fileData_      = {}; // 読み込まれたリプレイデータ全体の保持
    std::string filepath_     = ""; // 読み込み元のファイルパス
    size_t currentFrameIndex_ = 0; // 現在再生中のフレーム番号

    bool isActive_ = false; // 再生中（初期化済み）かどうかのフラグ

public:
    /// <summary> 再生中かどうかを取得する. </summary>
    bool GetIsActive() const { return isActive_; }

    /// <summary> 読み込み済みリプレイのファイルパスを取得する. </summary>
    const std::string& GetFilepath() const { return filepath_; }
    /// <summary> リプレイ開始時のシーン名を取得する. </summary>
    const std::string& GetStartSceneName() const { return fileData_.header.startScene; }

    /// <summary> リプレイの総フレーム数を取得する. </summary>
    size_t GetTotalFrameCount() const { return fileData_.frameData.size(); }
    /// <summary> 現在の再生フレーム番号を取得する. </summary>
    size_t GetCurrentFrameIndex() const { return currentFrameIndex_; }
    /// <summary> 現在の再生フレーム番号を直接設定する. </summary>
    void SetCurrentFrameIndex(size_t index) { currentFrameIndex_ = index; }

    /// <summary> 現在のフレームのデータを取得する. </summary>
    const ReplayFrameData& GetCurrentFrameData() const { return fileData_.frameData[currentFrameIndex_]; }
    /// <summary> 指定したインデックスのフレームデータを取得する. </summary>
    const ReplayFrameData& GetFrameData(int32_t _frameIndex) const {
        if (_frameIndex < 0 || _frameIndex >= fileData_.frameData.size()) {
            LOG_ERROR("Frame index out of range: {}", _frameIndex);
            static ReplayFrameData tmpData; // 安全のために static で返す
            return tmpData;
        }
        return fileData_.frameData[_frameIndex];
    }
};

} // namespace OriGine
