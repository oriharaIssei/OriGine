#pragma once

#ifdef _DEBUG

#include <string>

namespace OriGine {

class Emitter;
class Scene;

/// <summary>
/// Emitter の共通エディタ GUI を担当するクラス。
/// ParticleSystem・EntitySpawner など Emitter を所有するコンポーネントが共用する。
/// </summary>
class EmitterEditor {
public:
    /// <summary>
    /// Emitter の共通 UI を描画する。
    /// </summary>
    /// <returns>PlayStart() が呼ばれた場合 true（呼び出し元でリソース生成が必要な場合は対応すること）</returns>
    static bool Draw(Emitter& _ctrl, const std::string& _parentLabel, Scene* _scene);

private:
    static void EditShape(Emitter& _ctrl, const std::string& _parentLabel);
};

} // namespace OriGine

#endif // _DEBUG
