#include "AnimationData.h"

float CalculateValue::Linear(const std::vector<Keyframe<float>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return 0.f;
        }
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return std::lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

Vec2f CalculateValue::Linear(const std::vector<Keyframe<Vec2f>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Vec2f();
        }

        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Vec3f CalculateValue::Linear(const std::vector<KeyframeVector3>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Vec3f();
        }
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を
    return (*keyframes.rbegin()).value;
}
Vec4f CalculateValue::Linear(const std::vector<Keyframe<Vec4f>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Vec4f();
        }
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Quaternion CalculateValue::Linear(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Quaternion();
        }
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

float CalculateValue::Step(const std::vector<Keyframe<float>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return 0.f;
        }
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Vec2f CalculateValue::Step(const std::vector<Keyframe<Vec2f>>& keyframes, float time) {
    if (keyframes.empty()) {
        return Vec2f();
    }

    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Vec3f CalculateValue::Step(const std::vector<KeyframeVector3>& keyframes, float time) {
    if (keyframes.empty()) {
        return Vec3f();
    }
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Vec4f CalculateValue::Step(const std::vector<Keyframe<Vec4f>>& keyframes, float time) {
    if (keyframes.empty()) {
        return Vec4f();
    }
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Quaternion CalculateValue::Step(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    if (keyframes.empty()) {
        return Quaternion();
    }
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
