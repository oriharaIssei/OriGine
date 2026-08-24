#pragma once

/// stl
#include <cstdint>

namespace OriGine {

/// <summary>まだ採番されていない型IDを表す番兵. 添字として密に 0 から使うため、無効値は上限側に置く</summary>
inline constexpr uint32_t kInvalidComponentTypeId = 0xFFFFFFFFu;

/// <summary>
/// 同時に存在できるコンポーネント型の上限.
/// Phase 5 のクエリを uint64_t 1本のビットマスクで表せるように 64 にしてある.
/// 256 へ広げるときは、この定数とマスク側の型(uint64_t -> uint64_t[4] や std::bitset)を変える.
/// 型ID自体の表現は変わらないので、ここを増やしても呼び出し側には波及しない.
/// </summary>
inline constexpr uint32_t kMaxComponentTypes = 64;

/// <summary>
/// コンポーネント型ごとの型IDの置き場所.
///
/// なぜクラステンプレートなのか:
///   static メンバは「そのクラス1つにつき1個」しか実体を持たない。
///   IComponentArray のような非テンプレートの基底に置くと、派生した全てのコンポーネント型が
///   同じ1個の変数を共有してしまい、「型ごとに一意なID」にならない。
///   クラステンプレートにすれば ComponentTypeIdStorage<Transform> と
///   ComponentTypeIdStorage<SphereCollider> は別のクラスなので、型ごとに1個ずつ実体ができる。
///
/// なぜ inline static の定数初期化なのか:
///   1. 動的初期化子を持たないため、静的ライブラリでリンカに捨てられる心配がない
///      (静的初期化子による自己登録がこのビルドで機能しないのと同じ罠を避けている)。
///   2. 関数ローカル static を「関数呼び出しの戻り値」で初期化すると、MSVC はスレッドセーフ初期化の
///      ガード変数チェックを毎回の呼び出しに挿入する。この値は1フレームに20万回超読まれるため、
///      ガードの無い素のロードで済ませたい。
///
/// 値の意味は「型の identity」であって「どのシーンの何番目か」ではない。
/// だからシーンを作り直しても、配列から要素を消しても、この値は無効化されない。
/// </summary>
template <typename ComponentType>
struct ComponentTypeIdStorage {
    inline static uint32_t id_ = kInvalidComponentTypeId;
};

} // namespace OriGine
