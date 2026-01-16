#include "AnimationData.h"

using namespace OriGine;

float CalculateValue::Linear(const std::vector<Keyframe<float>>& _keyframes, float _time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (_keyframes.empty()) {
            return 0.f;
        }
        if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
            return _keyframes[0].value;
        }
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (_time - _keyframes[index].time) / (_keyframes[nextIndex].time - _keyframes[index].time);
            return std::lerp(_keyframes[index].value, _keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*_keyframes.rbegin()).value;
}

Vec2f CalculateValue::Linear(const std::vector<Keyframe<Vec2f>>& _keyframes, float _time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (_keyframes.empty()) {
            return Vec2f();
        }

        if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
            return _keyframes[0].value;
        }
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (_time - _keyframes[index].time) / (_keyframes[nextIndex].time - _keyframes[index].time);
            return Lerp(_keyframes[index].value, _keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*_keyframes.rbegin()).value;
}
Vec3f CalculateValue::Linear(const std::vector<KeyframeVector3>& _keyframes, float _time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (_keyframes.empty()) {
            return Vec3f();
        }
        if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
            return _keyframes[0].value;
        }
    }

    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (_time - _keyframes[index].time) / (_keyframes[nextIndex].time - _keyframes[index].time);
            return Lerp(_keyframes[index].value, _keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を
    return (*_keyframes.rbegin()).value;
}
Vec4f CalculateValue::Linear(const std::vector<Keyframe<Vec4f>>& _keyframes, float _time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (_keyframes.empty()) {
            return Vec4f();
        }
        if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
            return _keyframes[0].value;
        }
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (_time - _keyframes[index].time) / (_keyframes[nextIndex].time - _keyframes[index].time);
            return Lerp(_keyframes[index].value, _keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*_keyframes.rbegin()).value;
}
Quaternion CalculateValue::Linear(const std::vector<KeyframeQuaternion>& _keyframes, float _time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (_keyframes.empty()) {
            return Quaternion();
        }
        if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
            return _keyframes[0].value;
        }
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (_time - _keyframes[index].time) / (_keyframes[nextIndex].time - _keyframes[index].time);
            return Slerp(_keyframes[index].value, _keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*_keyframes.rbegin()).value;
}

float CalculateValue::Step(const std::vector<Keyframe<float>>& _keyframes, float _time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (_keyframes.empty()) {
            return 0.f;
        }
        if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
            return _keyframes[0].value;
        }
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            return _keyframes[index].value;
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*_keyframes.rbegin()).value;
}
Vec2f CalculateValue::Step(const std::vector<Keyframe<Vec2f>>& _keyframes, float _time) {
    if (_keyframes.empty()) {
        return Vec2f();
    }

    if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
        return _keyframes[0].value;
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            return _keyframes[index].value;
        }
    }
    return (*_keyframes.rbegin()).value;
}
Vec3f CalculateValue::Step(const std::vector<KeyframeVector3>& _keyframes, float _time) {
    if (_keyframes.empty()) {
        return Vec3f();
    }
    if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
        return _keyframes[0].value;
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            return _keyframes[index].value;
        }
    }
    return (*_keyframes.rbegin()).value;
}
Vec4f CalculateValue::Step(const std::vector<Keyframe<Vec4f>>& _keyframes, float _time) {
    if (_keyframes.empty()) {
        return Vec4f();
    }
    if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
        return _keyframes[0].value;
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            return _keyframes[index].value;
        }
    }
    return (*_keyframes.rbegin()).value;
}
Quaternion CalculateValue::Step(const std::vector<KeyframeQuaternion>& _keyframes, float _time) {
    if (_keyframes.empty()) {
        return Quaternion();
    }
    if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
        return _keyframes[0].value;
    }
    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            return _keyframes[index].value;
        }
    }
    return (*_keyframes.rbegin()).value;
}
