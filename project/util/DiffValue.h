#pragma once

/// <summary>
/// 現在値と前回値を保持し、差分も取得できるテンプレートクラス
/// </summary>
template <typename T>
class DiffValue{
public:
    DiffValue() = default;
    DiffValue(const T& value): current_(value),prev_(value){}

    /// 値を更新し、前回値を保存
    void set(const T& value){
        prev_ = current_;
        current_ = value;
    }

    /// 現在値を取得
    const T& current() const{ return current_; }

    /// 前回値を取得
    const T& prev() const{ return prev_; }

    /// 差分を取得
    T delta() const{ return current_ - prev_; }

    bool isChanged() const{
        return current_ != prev_;
    }
    bool isTrigger()const{
        return current_ && !prev_;
    }
    bool isRelease() const{
        return !current_ && prev_;
    }

    /// 前回値を現在値にリセット
    void sync(){ prev_ = current_; }

private:
    T current_{};
    T prev_{};
};
