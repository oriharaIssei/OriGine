#pragma once

/// <summary>
/// 現在値と前回値を保持し、差分も取得できるテンプレートクラス
/// </summary>
template <typename T>
class DiffValue {
public:
    DiffValue() = default;
    DiffValue(const T& _value) : current_(_value), prev_(_value) {}

    /// <summary>
    /// 値を更新し、前回値を保存
    /// </summary>
    /// <param name="value"></param>
    void Set(const T& _value) {
        prev_    = current_;
        current_ = _value;
    }

    /// <summary>
    /// 現在値を設定（前回値は変更しない）
    /// </summary>
    /// <param name="value"></param>
    void SetCurrent(const T& _value) {
        current_ = _value;
    }

    /// <summary>
    /// 現在値を取得
    /// </summary>
    /// <returns>現在値</returns>
    const T& Current() const { return current_; }

    /// <summary>
    /// 現在値の参照を取得
    /// </summary>
    /// <returns></returns>
    T& CurrentRef() { return current_; }

    /// <summary>
    /// 前回値を取得
    /// </summary>
    /// <returns>前回値</returns>
    const T& Prev() const { return prev_; }

    /// <summary>
    /// 前回値の参照を取得
    /// </summary>
    /// <returns></returns>
    T& PrevRef() { return prev_; }

    /// <summary>
    /// 差分を取得
    /// </summary>
    /// <returns>現在値 - 前回値</returns>
    T Delta() const { return current_ - prev_; }

    /// <summary>
    /// 値が変更されたか
    /// </summary>
    /// <returns>変更されていればtrue</returns>
    bool IsChanged() const {
        return current_ != prev_;
    }
    /// <summary>
    /// トリガー（falseからtrueへの変化）が発生したか
    /// </summary>
    /// <returns>トリガーされていればtrue</returns>
    bool IsTrigger() const {
        return current_ && !prev_;
    }
    /// <summary>
    /// リリース（trueからfalseへの変化）が発生したか
    /// </summary>
    /// <returns>リリースされていればtrue</returns>
    bool IsRelease() const {
        return !current_ && prev_;
    }

    /// <summary>
    /// 前回値を現在値に同期
    /// </summary>
    void Sync() { prev_ = current_; }

private:
    T current_{};
    T prev_{};
};
