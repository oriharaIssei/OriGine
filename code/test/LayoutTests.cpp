#include "test/LayoutTests.h"

/// stl
#include <format>
#include <string>
#include <string_view>
#include <vector>

/// ECS: 葉コンポーネント(54型)。並び順は EngineInclude.h の ENGINE_COMPONENTS の並びに合わせ、
/// そこに無い型(EntitySpawner / LineRenderer / ModelMeshRenderer)は末尾にまとめている。
#include "audio/Audio.h"

#include "component/EntityReferenceList.h"
#include "component/scene/SceneChanger.h"
#include "component/scene/SubScene.h"

#include "component/transform/CameraTransform.h"
#include "component/transform/Transform.h"
#include "component/transform/Transform2d.h"

#include "component/material/Material.h"
#include "component/material/light/DirectionalLight.h"
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"

#include "component/animation/DissolveAnimation.h"
#include "component/animation/MaterialAnimation.h"
#include "component/animation/ModelNodeAnimation.h"
#include "component/animation/PrimitiveNodeAnimation.h"
#include "component/animation/SkinningAnimationComponent.h"
#include "component/animation/SpriteAnimation.h"
#include "component/animation/TransformAnimation.h"
#include "component/animation/TransformRateAnimation.h"
#include "component/effect/CameraAction.h"

#include "component/collision/CollisionPushBackInfo.h"
#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/CapsuleCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/RayCollider.h"
#include "component/collision/collider/SegmentCollider.h"
#include "component/collision/collider/SphereCollider.h"
#include "component/collision/collider/base/Collider.h"

#include "component/physics/Rigidbody.h"

#include "component/effect/MaterialEffectPipeLine.h"
#include "component/effect/SquashStretchComponent.h"
#include "component/effect/particle/emitter/ParticleSystem.h"
#include "component/effect/particle/gpuParticle/GpuParticle.h"
#include "component/effect/post/DissolveEffectParam.h"
#include "component/effect/post/DistortionEffectParam.h"
#include "component/effect/post/GradationComponent.h"
#include "component/effect/post/GrayscaleComponent.h"
#include "component/effect/post/OutlineComponent.h"
#include "component/effect/post/RadialBlurParam.h"
#include "component/effect/post/RandomEffectParam.h"
#include "component/effect/post/SmoothingEffectParam.h"
#include "component/effect/post/SpeedlineEffectParam.h"
#include "component/effect/post/VignetteParam.h"

#include "component/text/TextComponent.h"
#include "component/text/TextStreamComponent.h"

#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"
#include "component/renderer/SkyboxRenderer.h"
#include "component/renderer/Sprite.h"

// EngineInclude.h の ENGINE_COMPONENTS には無いが、IComponent 派生の葉型である3つ
// (Phase 3 計画 付録A: 「葉は54型」に含まれる。理由は本ファイル末尾のコメントを参照)
#include "component/renderer/LineRenderer.h"
#include "component/renderer/ModelMeshRenderer.h"
#include "component/spawner/EntitySpawner.h"

using namespace OriGine;

namespace OriGine::Test {

namespace {

/// <summary>
/// 1型分の sizeof / alignof を診断行へ積む。
/// テンプレート引数にコンマを含む型(MeshRenderer&lt;A, B&gt; 等)でもマクロと違い
/// プリプロセッサのコンマ分割を気にせず書けるのが、関数テンプレートにしている理由。
/// </summary>
template <typename T>
void AppendRow(std::vector<std::string>& _lines, std::string_view _label) {
    _lines.push_back(std::format("{:<42} sizeof={:<6} alignof={}", _label, sizeof(T), alignof(T)));
}

/// <summary>
/// ケース1: コンポーネントの葉54型。
/// </summary>
TestCaseResult LeafComponentSizes() {
    TestCaseResult result;

    AppendRow<Audio>(result.diagnosticLines, "Audio");

    AppendRow<EntityReferenceList>(result.diagnosticLines, "EntityReferenceList");
    AppendRow<SceneChanger>(result.diagnosticLines, "SceneChanger");
    AppendRow<SubScene>(result.diagnosticLines, "SubScene");

    AppendRow<CameraTransform>(result.diagnosticLines, "CameraTransform");
    AppendRow<Transform>(result.diagnosticLines, "Transform");
    AppendRow<Transform2d>(result.diagnosticLines, "Transform2d");

    AppendRow<Material>(result.diagnosticLines, "Material");
    AppendRow<DirectionalLight>(result.diagnosticLines, "DirectionalLight");
    AppendRow<PointLight>(result.diagnosticLines, "PointLight");
    AppendRow<SpotLight>(result.diagnosticLines, "SpotLight");

    // #ifdef _DEBUG / #ifdef DEBUG でフィールドが変わる5型(Develop/Debugで値が変わりうる)
    AppendRow<DissolveAnimation>(result.diagnosticLines, "DissolveAnimation [#ifdef]");
    AppendRow<MaterialAnimation>(result.diagnosticLines, "MaterialAnimation [#ifdef]");
    AppendRow<ModelNodeAnimation>(result.diagnosticLines, "ModelNodeAnimation");
    AppendRow<PrimitiveNodeAnimation>(result.diagnosticLines, "PrimitiveNodeAnimation");
    AppendRow<SkinningAnimationComponent>(result.diagnosticLines, "SkinningAnimationComponent");
    AppendRow<SpriteAnimation>(result.diagnosticLines, "SpriteAnimation [#ifdef]");
    AppendRow<TransformAnimation>(result.diagnosticLines, "TransformAnimation [#ifdef]");
    AppendRow<TransformRateAnimation>(result.diagnosticLines, "TransformRateAnimation");
    AppendRow<CameraAction>(result.diagnosticLines, "CameraAction");

    AppendRow<AABBCollider>(result.diagnosticLines, "AABBCollider");
    AppendRow<CapsuleCollider>(result.diagnosticLines, "CapsuleCollider");
    AppendRow<OBBCollider>(result.diagnosticLines, "OBBCollider");
    AppendRow<RayCollider>(result.diagnosticLines, "RayCollider");
    AppendRow<SegmentCollider>(result.diagnosticLines, "SegmentCollider");
    AppendRow<SphereCollider>(result.diagnosticLines, "SphereCollider");
    AppendRow<CollisionPushBackInfo>(result.diagnosticLines, "CollisionPushBackInfo");

    AppendRow<Rigidbody>(result.diagnosticLines, "Rigidbody");

    AppendRow<MaterialEffectPipeLine>(result.diagnosticLines, "MaterialEffectPipeLine");
    AppendRow<SquashStretchComponent>(result.diagnosticLines, "SquashStretchComponent");
    AppendRow<ParticleSystem>(result.diagnosticLines, "ParticleSystem [#ifdef]");
    AppendRow<GpuParticleEmitter>(result.diagnosticLines, "GpuParticleEmitter");
    AppendRow<DissolveEffectParam>(result.diagnosticLines, "DissolveEffectParam");
    AppendRow<DistortionEffectParam>(result.diagnosticLines, "DistortionEffectParam");
    AppendRow<GradationComponent>(result.diagnosticLines, "GradationComponent");
    AppendRow<GrayscaleComponent>(result.diagnosticLines, "GrayscaleComponent");
    AppendRow<OutlineComponent>(result.diagnosticLines, "OutlineComponent");
    AppendRow<RadialBlurParam>(result.diagnosticLines, "RadialBlurParam");
    AppendRow<RandomEffectParam>(result.diagnosticLines, "RandomEffectParam");
    AppendRow<SmoothingEffectParam>(result.diagnosticLines, "SmoothingEffectParam");
    AppendRow<SpeedlineEffectParam>(result.diagnosticLines, "SpeedlineEffectParam");
    AppendRow<VignetteParam>(result.diagnosticLines, "VignetteParam");

    AppendRow<TextComponent>(result.diagnosticLines, "TextComponent");
    AppendRow<TextStreamComponent>(result.diagnosticLines, "TextStreamComponent");

    AppendRow<BoxRenderer>(result.diagnosticLines, "BoxRenderer");
    AppendRow<CylinderRenderer>(result.diagnosticLines, "CylinderRenderer");
    AppendRow<PlaneRenderer>(result.diagnosticLines, "PlaneRenderer");
    AppendRow<RingRenderer>(result.diagnosticLines, "RingRenderer");
    AppendRow<SphereRenderer>(result.diagnosticLines, "SphereRenderer");
    AppendRow<SkyboxRenderer>(result.diagnosticLines, "SkyboxRenderer");
    AppendRow<SpriteRenderer>(result.diagnosticLines, "SpriteRenderer");
    AppendRow<LineRenderer>(result.diagnosticLines, "LineRenderer");
    AppendRow<ModelMeshRenderer>(result.diagnosticLines, "ModelMeshRenderer");

    AppendRow<EntitySpawner>(result.diagnosticLines, "EntitySpawner");

    result.diagnosticLines.push_back(std::format("-- {} leaf type(s) listed --", 54));

    return result;
}

/// <summary>
/// ケース2: 中間クラス(IComponent / ICollider / Collider&lt;Bounds&gt;各実体 / MeshRenderer系 /
/// PrimitiveMeshRendererBase)。
/// </summary>
TestCaseResult IntermediateClassSizes() {
    TestCaseResult result;

    AppendRow<IComponent>(result.diagnosticLines, "IComponent");
    AppendRow<ICollider>(result.diagnosticLines, "ICollider");

    AppendRow<Collider<Bounds::AABB>>(result.diagnosticLines, "Collider<Bounds::AABB>");
    AppendRow<Collider<Bounds::Sphere>>(result.diagnosticLines, "Collider<Bounds::Sphere>");
    AppendRow<Collider<Bounds::OBB>>(result.diagnosticLines, "Collider<Bounds::OBB>");
    AppendRow<Collider<Bounds::Capsule>>(result.diagnosticLines, "Collider<Bounds::Capsule>");
    AppendRow<Collider<Bounds::Segment>>(result.diagnosticLines, "Collider<Bounds::Segment>");
    AppendRow<Collider<Bounds::Ray>>(result.diagnosticLines, "Collider<Bounds::Ray>");

    // MeshRenderer<MeshTemplate, VertexDataType> の実際のインスタンス化4種
    // (各派生の宣言から拾った組み合わせ。派生クラスは実体を追加で持たないことが多いので、
    //  ここでのサイズは対応する葉型とほぼ一致するはず)
    AppendRow<MeshRenderer<Mesh<ColorVertexData>, ColorVertexData>>(
        result.diagnosticLines, "MeshRenderer<Mesh<ColorVertexData>,...> (LineRendererの基底)");
    AppendRow<MeshRenderer<TextureColorMesh, TextureColorVertexData>>(
        result.diagnosticLines, "MeshRenderer<TextureColorMesh,...> (ModelMeshRenderer/PrimitiveMeshRendererBaseの基底)");
    AppendRow<MeshRenderer<Mesh<SkyboxVertex>, SkyboxVertex>>(
        result.diagnosticLines, "MeshRenderer<Mesh<SkyboxVertex>,...> (SkyboxRendererの基底)");
    AppendRow<MeshRenderer<SpriteMesh, SpriteVertexData>>(
        result.diagnosticLines, "MeshRenderer<SpriteMesh,...> (SpriteRendererの基底)");

    AppendRow<PrimitiveMeshRendererBase>(result.diagnosticLines, "PrimitiveMeshRendererBase");

    // テンプレート基底3つ目(Collider<>, MeshRenderer<>に続く)。5型のうち代表として1つだけ載せる
    // (BoxRenderer/CylinderRenderer/... は形状パラメータが違うだけで、レイアウトに影響する
    //  追加フィールドをPrimitiveMeshRenderer<PrimType>自体には持たないため)
    AppendRow<PrimitiveMeshRenderer<Primitive::Box>>(result.diagnosticLines, "PrimitiveMeshRenderer<Primitive::Box>");

    return result;
}

} // namespace

std::vector<TestCaseEntry> MakeLayoutTestCases() {
    return {
        {"LeafComponentSizes", LeafComponentSizes},
        {"IntermediateClassSizes", IntermediateClassSizes},
    };
}

} // namespace OriGine::Test

// 葉54型の内訳について:
// EngineInclude.h の ENGINE_COMPONENTS マクロが束ねているインクルード一覧には
// EntitySpawner / LineRenderer / ModelMeshRenderer の3つが含まれていない(未着手機能のため
// FrameWork.cpp では登録されていない)。それでもIComponent派生の具象型として存在し、
// 「コンポーネントの葉は54型」という付録Aの前提に含まれているため、本ファイルでは
// EngineInclude.h に頼らずこの3つを直接インクルードしている。
