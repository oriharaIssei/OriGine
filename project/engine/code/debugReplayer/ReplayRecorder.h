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
/// デバッグのプレイ情報を記録するクラス
/// </summary>
class ReplayRecorder {
public:
    ReplayRecorder();
    ~ReplayRecorder();

    void Initialize(const std::string& _startSceneName);
    void Finalize();

    void RecordFrame(float deltaTime, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput);
    bool SaveToFile(const std::string& _directory, const std::string& _filename = TimeToString());

private:
    /// <summary>
    /// 自らが持つヘッダー情報を書き込む
    /// </summary>
    /// <param name="_ofs"></param>
    void WriteHeader(std::ofstream& _ofs);
    /// <summary>
    /// 指定したフレームのデータを書き込む
    /// </summary>
    /// <param name="_ofs"></param>
    /// <param name="_frameIndex"></param>
    void WriteFrameData(std::ofstream& _ofs, size_t _frameIndex);

private:
    ReplayFileHeader header_             = {};
    std::vector<ReplayFrameData> frames_ = {};
};

} // namespace OriGine
