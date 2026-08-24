#pragma once

/// stl
#include <array>
#include <cstddef>
#include <cstdint>

/// engine
#include "EngineConfig.h"

namespace OriGine::CallCounter {

/// <summary>
/// 計測点(呼び出し箇所)を識別するハンドル。Register()が返す内部配列の添字そのものであり、
/// Increment()はこの値を毎回の名前解決なしに配列インデックスとして使う。
/// </summary>
using Handle = int32_t;

/// <summary>Register()が上限超過で失敗した際に返る無効値。Increment(kInvalidHandle)は安全に何もしない。</summary>
constexpr Handle kInvalidHandle = -1;

/// <summary>
/// 登録できる計測点の異なり数の上限。
/// ComponentRepository::GetComponentArray&lt;T&gt;() はコンポーネント型ごとに、
/// ComponentArray&lt;T&gt;側の各関数は「関数単位」で(型をまたいで同じカウンタへ集約して)
/// 登録される想定のため、実際の消費数は概ね「コンポーネント型数 + ComponentArrayの対象関数数」程度に収まる。
/// 超過時はRegisterが-1を返し、その計測点だけ記録されない(値を黙って捨てず、LOG_WARNを1度だけ出す)。
/// </summary>
constexpr size_t kMaxCounters = 256;

/// <summary>
/// 呼び出し回数を記録する名前を登録し、以後この名前専用のハンドルを返す。
///
/// 同じ名前で複数回呼ばれた場合(=ComponentArray&lt;T&gt;のようにComponentType毎に実体化される
/// テンプレート関数が、型をまたいで同一の名前でRegisterしてくる場合)は、新規に枠を消費せず
/// 既存のハンドルを返す。呼び出し元ごとに別々のカウンタが増えてしまうと、
/// 「この関数が型を問わず何回呼ばれたか」という関数単位の合計が見えなくなるうえ、
/// コンポーネント型数 × 対象関数数の組み合わせで簡単に kMaxCounters を超えてしまうため、
/// この重複排除は省略できない。
///
/// ホットパスではなく、呼び出し側の `static Handle h = Register(_name);` という
/// 1度きりの静的初期化から呼ばれる想定。そのため文字列比較を伴う線形探索を行っても
/// (登録数はkMaxCounters=256が上限のため)実用上問題にならない。
/// </summary>
/// <param name="_name">計測点の名前(表示上は EngineConfig::Profiler::kEventNameCapacity で切り詰められる)</param>
/// <returns>以後Incrementに渡すハンドル。上限超過時はkInvalidHandle</returns>
Handle Register(const char* _name);

/// <summary>
/// Increment()がインライン展開されたホットパスから直接読み書きするための実カウンタ配列の実体。
/// ヘッダのinline関数から参照できるよう外部リンケージにし、実体はCallCounter.cppという
/// 単一の翻訳単位にのみ置く(複数TUに実体を持たせるとODR違反になる)。
/// Register/Increment/OnFrameBegin以外からは触らないこと。
/// </summary>
extern std::array<uint64_t, kMaxCounters> g_rawCounts;

/// <summary>
/// 登録済みハンドルの呼び出し回数を1増やす。1フレームに25万回超呼ばれるホットパスであるため、
/// 素のメモリインクリメント1回に落ちることを最優先にした設計にしている。
///
/// - atomicにしない: 現状 ISystem::Run 系のシステム更新はシングルスレッドで呼ばれる
///   (SystemRunner.cppにスレッド生成が無いことを確認済み)。x86のlock付きRMW
///   (std::atomic::fetch_add等)は非競合でも約20サイクルかかり、25万回/フレーム×約7ns
///   ≒ 1.8msの上乗せになる。これはこの計測で見ようとしている効果量そのものと同オーダーであり、
///   計測器が計測対象を破壊してしまう。Phase 9でシステム更新が並列化されたら、ここは
///   スレッドローカルな加算バッファ+フレーム末尾での集約に置き換える必要がある
///   (このままだとデータ競合になる)。
/// - 名前解決をしない: 呼び出し側が `static Handle h = Register(name)` により
///   名前解決を初回の1度だけで済ませ、ここには整数ハンドルしか渡ってこない前提にしている。
/// </summary>
/// <param name="_handle">Register()が返したハンドル。kInvalidHandleなら何もしない</param>
inline void Increment(Handle _handle) {
    if (_handle < 0) {
        return;
    }
    ++g_rawCounts[static_cast<size_t>(_handle)];
}

/// <summary>1フレーム分の呼び出し回数統計(GetLastFrameStatsが返す配列の要素)</summary>
struct FrameStat {
    char name_[OriGine::Config::Profiler::kEventNameCapacity]{}; // 計測点の表示名(固定長・必要なら切り詰め済み)
    uint64_t count_ = 0; // 直前フレームでの呼び出し回数
};

/// <summary>
/// フレーム境界処理。累積カウンタから前フレーム時点の累積値を差し引いて、直前フレーム分の
/// 呼び出し回数を確定させる(AllocationCounter::OnFrameBeginと同じ「差分を取る」設計)。
/// OriGine::Profiler::BeginFrame() から1フレームに1回呼び出される。
/// </summary>
void OnFrameBegin();

/// <summary>
/// 直前フレームの全カウンタの統計を取得する。
/// </summary>
/// <param name="_outStats">統計配列の先頭を指すポインタの出力先</param>
/// <returns>_outStatsが指す配列の要素数(=これまでに登録されたカウンタ数)</returns>
size_t GetLastFrameStats(const FrameStat** _outStats);

} // namespace OriGine::CallCounter

// ============================================================================
// PROFILE_COUNT マクロ
//
// _RELEASE では PROFILE_SCOPE と同じ理由で完全に消える。
// 加えて ORIGINE_DISABLE_CALL_COUNTERS を定義すると Debug/Develop でも消える。
// これは「計測器を入れる前に記録したベースライン」と直接比較する際、計測器自身の
// オーバーヘッド分だけ嘘の差が乗ってしまうのを避けるための脱出ハッチ。
// ORIGINE_CALL_COUNTER_ENABLED は同じ条件を呼び出し側(例:
// ComponentRepository::GetComponentArray<T>()のカウンタ名組み立て)でも再利用するためのフラグ。
// ============================================================================
#if defined(_RELEASE) || defined(ORIGINE_DISABLE_CALL_COUNTERS)
#define ORIGINE_CALL_COUNTER_ENABLED 0
#define PROFILE_COUNT(_name) ((void)0)
#else
#define ORIGINE_CALL_COUNTER_ENABLED 1

// PROFILE_SCOPE(Profiler.h)のOriGine_PROFILE_CONCATと同じ「__LINE__で識別子を一意にする」トリック。
// CallCounterはAllocationCounterと同じ階層(Profilerが依存する側)に置きたいため、
// Profiler.hへ逆依存させないようここで独立に定義している。
#define OriGine_CALLCOUNTER_CONCAT_INNER(a, b) a##b
#define OriGine_CALLCOUNTER_CONCAT(a, b) OriGine_CALLCOUNTER_CONCAT_INNER(a, b)

/// <summary>
/// 現在行の呼び出しを1回数える。名前解決(Register)はstatic初期化により初回の1度だけ行われ、
/// 以降はハンドル経由の配列添字アクセスだけになる。
/// 例: PROFILE_COUNT("ComponentArray::GetComponents");
/// </summary>
#define PROFILE_COUNT(_name) \
    static ::OriGine::CallCounter::Handle OriGine_CALLCOUNTER_CONCAT(origine_callCounter_, __LINE__) = ::OriGine::CallCounter::Register(_name); \
    ::OriGine::CallCounter::Increment(OriGine_CALLCOUNTER_CONCAT(origine_callCounter_, __LINE__))
#endif
