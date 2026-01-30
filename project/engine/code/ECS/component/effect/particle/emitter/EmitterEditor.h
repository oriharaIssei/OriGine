#pragma once

#ifdef _DEBUG

#include <string>

namespace OriGine {

// 前方宣言
class Emitter;
class Scene;
struct EntityHandle;

/// <summary>
/// Emitter のエディタGUI機能を担当するクラス
/// </summary>
class EmitterEditor {
public:
    /// <summary>
    /// Emitter のエディタGUIを描画する
    /// </summary>
    /// <param name="_emitter">編集対象のEmitter</param>
    /// <param name="_scene">シーン</param>
    /// <param name="_entity">エンティティハンドル</param>
    /// <param name="_parentLabel">ラベルの親識別子</param>
    static void Draw(Emitter& _emitter, Scene* _scene, EntityHandle _entity, const std::string& _parentLabel);

private:
    /// <summary>
    /// Emitter の 基本設定項目を編集する
    /// </summary>
    static void EditEmitter(Emitter& _emitter, const std::string& _parentLabel);

    /// <summary>
    /// ShapeType に関する 編集項目
    /// </summary>
    static void EditShapeType(Emitter& _emitter, const std::string& _parentLabel);

    /// <summary>
    /// Particle の 編集項目
    /// </summary>
    static void EditParticle(Emitter& _emitter, const std::string& _parentLabel);

    /// <summary>
    /// Velocity の 編集項目
    /// </summary>
    static void EditVelocity(Emitter& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// Scale の 編集項目
    /// </summary>
    static void EditScale(Emitter& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// Rotate の 編集項目
    /// </summary>
    static void EditRotate(Emitter& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// Color の 編集項目
    /// </summary>
    static void EditColor(Emitter& _emitter, const std::string& _parentLabel, int32_t& _newFlag);

    /// <summary>
    /// UV の 編集項目
    /// </summary>
    static void EditUV(Emitter& _emitter, const std::string& _parentLabel, int32_t& _newFlag);
};

} // namespace OriGine

#endif // _DEBUG
