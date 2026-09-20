#pragma once

/// stl
#include <memory>

/// engine
// shape
#include "EmitterShape.h"
// component
#include "component/ComponentHandle.h"

/// externals
#include <nlohmann/json.hpp>

/// math
#include "math/Vector3.h"

namespace OriGine {

// 前方宣言
class ParticleSystem;
class EntitySpawner;
class EmitterEditor;
class EmitterShapeRenderingSystem;
class Scene;
struct Transform;

/// <summary>
/// 汎用スポーンコントローラ
/// IComponent ではない。ParticleSystem や Spawner が所有する。
/// スポーンの「タイミング・数・形状」を管理し、Update() で今フレームのスポーン数を返す。
/// </summary>
class Emitter {
    friend void to_json(nlohmann::json& _j, const Emitter& _ctrl);
    friend void from_json(const nlohmann::json& _j, Emitter& _ctrl);

    // オーナークラス：private メンバへ直接アクセス可（CalculateMaxSize, SpawnParticle など）
    friend class ParticleSystem;
    friend class EntitySpawner;

    // エディタ：ImGui / SetterCommand でフィールドの生ポインタが必要なため
#ifdef ORIGINE_EDITOR_ENABLED
    friend class EmitterEditor;
#endif
    friend class EmitterShapeRenderingSystem;

public:
    Emitter();
    ~Emitter() = default;

    /// <summary>
    /// 初期化（シェイプのデフォルト生成・タイマーリセット）
    /// </summary>
    void Initialize();

    /// <summary>
    /// 毎フレーム呼ぶ。今フレームのスポーン数を返す。
    /// </summary>
    /// <param name="_deltaTime">デルタタイム</param>
    /// <returns>スポーンすべき数。0 ならスポーン不要</returns>
    int32_t Update(float _deltaTime);

    /// <summary>
    /// アクティブかどうかを返す
    /// </summary>
    bool IsActive() const { return isActive_; }

    /// <summary>
    /// 非アクティブ化（タイマーはリセットしない）
    /// </summary>
    void Deactivate() { isActive_ = false; }

    /// <summary>
    /// activeTime が切れ、スポーンが終了したか（ループ時は常に false）
    /// </summary>
    bool IsExpired() const { return !isLoop_ && leftActiveTime_ <= 0.f; }

    /// <summary>
    /// スポーン位置をシェイプから取得
    /// </summary>
    Vec3f GetSpawnPos() const;

    /// <summary>
    /// 補間スポーン原点を返す。
    /// interpolateSpawnPos_ が true のとき preWorldOriginPos_ → worldOriginPos_ を _index/_total で線形補間する。
    /// false のときは worldOriginPos_ をそのまま返す。
    /// </summary>
    /// <param name="_index">今フレームのスポーンインデックス（0-based）</param>
    /// <param name="_total">今フレームの合計スポーン数</param>
    Vec3f GetInterpolatedOriginPos(int32_t _index, int32_t _total) const;

    /// <summary>
    /// worldOriginPos_ を更新する。毎スポーン前に呼ぶ。
    /// parent_ がある場合はそのワールド座標 + originPos_（オフセット）で算出する。
    /// </summary>
    void UpdateWorldOriginPos();

    // ── 親子関係 ──────────────────────────────────────────────

    Transform* GetParent() const { return parent_; }
    void SetParent(Transform* _parent) { parent_ = _parent; }

    const ComponentHandle& GetParentHandle() const { return parentHandle_; }
    void SetParentHandle(const ComponentHandle& _handle) { parentHandle_ = _handle; }

    /// <summary>
    /// parentHandle_ から parent_ (キャッシュポインタ) を解決する。
    /// Initialize および Editor での設定変更後に呼ぶ。
    /// </summary>
    void ResolveParent(Scene* _scene);

    // ── 再生制御 ──────────────────────────────────────────────

    /// <summary>
    /// 最初から再生開始（タイマーをリセットしてアクティブ化）
    /// </summary>
    void PlayStart();
    /// <summary>
    /// 途中から再生再開
    /// </summary>
    void PlayContinue();
    /// <summary>
    /// 再生停止
    /// </summary>
    void PlayStop();

private:
    /// <summary>
    /// spawnShape_ が未生成(shapeType_ 変更直後など)の場合に、shapeType_ に応じた形状を生成する
    /// </summary>
    void EnsureShape();

private:
    bool isActive_            = false; // 現在アクティブ(スポーン処理中)かどうか
    bool isLoop_              = false; // activeTime_ 経過後も繰り返しスポーンし続けるか
    bool interpolateSpawnPos_ = false; // 1フレーム内の複数スポーンをスポーン原点の移動に沿って補間するか

    float activeTime_     = 0.f; // アクティブ状態を維持する時間(秒)。isLoop_ が false のとき使用
    float leftActiveTime_ = 0.f; // activeTime_ の残り時間(秒)。0 以下で IsExpired() が true になる

    int32_t spawnCount_    = 1; // 1回のクールタイム経過ごとにスポーンする数
    float spawnCoolTime_   = 0.f; // 次のスポーンまでのクールタイム(秒)
    float currentCoolTime_ = 0.f; // クールタイムの残り時間(秒)

    Vec3f originPos_         = {};
    Vec3f worldOriginPos_    = {};
    Vec3f preWorldOriginPos_ = {};

    EmitterShapeType shapeType_ = EmitterShapeType::SPHERE;
    std::shared_ptr<EmitterShape> spawnShape_;

    // originPos_ はワールド空間でのオフセット。
    // worldOriginPos_ = parent_->GetWorldTranslate() + originPos_
    ComponentHandle parentHandle_; // シリアライズ・エディタ設定用
    Transform* parent_ = nullptr; // ランタイムキャッシュ（ResolveParent で更新）
};

void to_json(nlohmann::json& _j, const Emitter& _ctrl);
void from_json(const nlohmann::json& _j, Emitter& _ctrl);

} // namespace OriGine
