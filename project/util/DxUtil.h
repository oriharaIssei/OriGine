#pragma once

#include <wrl.h>

/// =========================================================
// DirectX / COM ユーティリティ
/// =========================================================

/// <summary>
/// COM オブジェクトの参照カウントを取得する
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="ptr"></param>
/// <returns></returns>
template <typename T>
ULONG GetComRefCount(const Microsoft::WRL::ComPtr<T>& _ptr) {
    if (!_ptr) {
        return 0;
    }

    // AddRef() が現在の参照カウント +1 を返す
    ULONG count = _ptr->AddRef();
    // 1 増やした分を戻す
    count = _ptr->Release();

    return count;
}
