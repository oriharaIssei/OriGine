#pragma once

#ifdef _DEBUG

#include <string>

namespace OriGine {

// 前方宣言
class ParticleSystem;
class Scene;
struct EntityHandle;

/// <summary>
/// ParticleSystem のエディタGUI機能を担当するクラス
/// </summary>
class ParticleSystemEditor {
public:
    /// <summary>
    /// ParticleSystem のエディタGUIを描画する
    /// </summary>
    /// <param name="_emitter">編集対象のParticleSystem</param>
    /// <param name="_scene">シーン</param>
    /// <param name="_entity">エンティティハンドル</param>
    /// <param name="_parentLabel">ラベルの親識別子</param>
    static void Draw(ParticleSystem& _emitter, Scene* _scene, EntityHandle _entity, const std::string& _parentLabel);

private:
    /// <summary>
    /// ParticleSystem の 基本設定項目を編集する
    /// </summary>
    static void EditEmitter(ParticleSystem& _emitter, const std::string& _parentLabel);

    /// <summary>
    /// Particle の 編集項目
    /// </summary>
    static void EditParticle(ParticleSystem& _emitter, const std::string& _parentLabel);

    /// <summary>
    /// Velocity の 編集項目
    /// </summary>
    static void EditVelocity(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// Scale の 編集項目
    /// </summary>
    static void EditScale(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// Rotate の 編集項目
    /// </summary>
    static void EditRotate(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// Color の 編集項目
    /// </summary>
    static void EditColor(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// UV の 編集項目
    /// </summary>
    static void EditUV(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag);
};

} // namespace OriGine

#endif // _DEBUG
