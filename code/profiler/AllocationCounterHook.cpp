#include "AllocationCounter.h"

/// stl
#include <cstdlib>
#include <new>

// ============================================================================
// グローバル operator new/delete の差し替え本体(Phase 4 4D D-4)。
//
// このファイルは OriGine.dll だけでなく、ECS_TestGame.exe / ECS_TestEditor.exe の
// 各プロジェクトにも「同じソースファイルとして」コンパイルされる(premake の files に
// 直接列挙してある)。理由: グローバル operator new/delete の差し替えはモジュール単位
// (DLLならDLL、EXEならEXE)でしか効かない。DLL 側だけに差し替えを置くと、EXE 側の
// コードが行う確保(例: main.cpp や FrameWork.cpp が直接 new する分)は素通しの
// ucrtbase.dll 既定の operator new/delete に行ってしまい、数えられなくなる
// (docs/plans/phase-04.md 6章の罠1: 確保回数が減ったら速くなったのではなく壊れている)。
//
// 一方で「今何バイト確保中か・ピークは何バイトか・合計何回確保したか」という状態は
// プロセス全体で1つに集約したい(Profiler/ProfilerWindow が読むのはその合計値のため)。
// そのため集計そのもの(atomicなカウンタ)は AllocationCounter.cpp に1つだけ置き、
// ここではそれを呼び出すだけにしてある(OriGine::AllocationCounter::RecordAlloc/RecordFree、
// ORIGINE_API でエクスポートされているので EXE 側からも呼べる)。
//
// 差し替え本体は `#if !defined(_RELEASE)` で丸ごとガードされている(元の
// AllocationCounter.cpp と同じ条件)。Release構成ではこのファイル内に operator new/delete の
// 定義が一切現れないため、リンク時にCRT標準の operator new/delete がそのまま採用される。
// ============================================================================

#if !defined(_RELEASE)

namespace {

/// <summary>
/// 確保したブロックの直前に埋め込むヘッダ.
/// sized/unsizedどちらのdelete呼び出しでも正確なサイズと解放先を得られるようにするため、
/// コンパイラのsized delete対応有無に依存せず常にこのヘッダを頼りに解放する.
/// </summary>
struct AllocHeader {
    size_t size_;   // ユーザーが要求したバイト数
    size_t offset_; // malloc の生ポインタからユーザーポインタまでのオフセット(free時に生ポインタへ戻すため)
};

constexpr size_t kMinAlignment = alignof(std::max_align_t);

/// <summary>
/// 追跡付きのメモリ確保. 要求バイト数 + ヘッダ + アライメント調整分を malloc で確保し、
/// ユーザーへ返すポインタの直前にヘッダを埋め込む.
/// </summary>
void* TrackedAllocate(size_t _size, size_t _align, bool _noThrow) {
    if (_align < kMinAlignment) {
        _align = kMinAlignment;
    }

    const size_t headerSize = sizeof(AllocHeader);
    const size_t totalSize  = _size + headerSize + _align;

    void* raw = std::malloc(totalSize);
    if (!raw) {
        if (_noThrow) {
            return nullptr;
        }
        throw std::bad_alloc();
    }

    const uintptr_t rawAddr  = reinterpret_cast<uintptr_t>(raw);
    const uintptr_t userAddr = (rawAddr + headerSize + _align - 1) & ~(static_cast<uintptr_t>(_align) - 1);

    AllocHeader* header = reinterpret_cast<AllocHeader*>(userAddr) - 1;
    header->size_        = _size;
    header->offset_       = static_cast<size_t>(userAddr - rawAddr);

    OriGine::AllocationCounter::RecordAlloc(_size);

    return reinterpret_cast<void*>(userAddr);
}

/// <summary>
/// 追跡付きのメモリ解放. ヘッダからサイズと生ポインタを復元して std::free する.
/// </summary>
void TrackedFree(void* _ptr) noexcept {
    if (!_ptr) {
        return;
    }

    AllocHeader* header      = reinterpret_cast<AllocHeader*>(_ptr) - 1;
    const size_t size        = header->size_;
    const uintptr_t rawAddr  = reinterpret_cast<uintptr_t>(_ptr) - header->offset_;

    OriGine::AllocationCounter::RecordFree(size);

    std::free(reinterpret_cast<void*>(rawAddr));
}

} // namespace

// --- 非aligned / non-nothrow ---
void* operator new(std::size_t _size) {
    return TrackedAllocate(_size, kMinAlignment, false);
}
void* operator new[](std::size_t _size) {
    return TrackedAllocate(_size, kMinAlignment, false);
}

// --- 非aligned / nothrow ---
void* operator new(std::size_t _size, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, kMinAlignment, true);
}
void* operator new[](std::size_t _size, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, kMinAlignment, true);
}

// --- 非aligned delete ---
void operator delete(void* _ptr) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, std::size_t) noexcept {
    // サイズはヘッダから復元するため引数は使用しない(sized delete対応)
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::size_t) noexcept {
    TrackedFree(_ptr);
}

// --- aligned / non-nothrow ---
void* operator new(std::size_t _size, std::align_val_t _align) {
    return TrackedAllocate(_size, static_cast<size_t>(_align), false);
}
void* operator new[](std::size_t _size, std::align_val_t _align) {
    return TrackedAllocate(_size, static_cast<size_t>(_align), false);
}

// --- aligned / nothrow ---
void* operator new(std::size_t _size, std::align_val_t _align, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, static_cast<size_t>(_align), true);
}
void* operator new[](std::size_t _size, std::align_val_t _align, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, static_cast<size_t>(_align), true);
}

// --- aligned delete ---
void operator delete(void* _ptr, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, std::size_t, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::size_t, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}

#endif // !defined(_RELEASE)
