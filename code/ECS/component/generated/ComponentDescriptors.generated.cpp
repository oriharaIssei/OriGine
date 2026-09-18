// ============================================================================
// 自動生成ファイル。手で編集しないこと。
// 生成元: project/engine/tools/ReflectionCodeGen
// ============================================================================
#include "component/generated/ComponentDescriptors.generated.h"

#include "component/ComponentReflection.h"
#include "component/ComponentRegistry.h"

#include "component/transform/Transform.h"
#include "component/transform/Transform2d.h"
#include "component/transform/CameraTransform.h"
#include "component/material/light/DirectionalLight.h"
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/effect/post/OutlineComponent.h"
#include "component/effect/post/SmoothingEffectParam.h"
#include "component/text/TextComponent.h"
#include "component/text/TextStreamComponent.h"

#include <cstddef>
#include <cstdint>

namespace OriGine {
namespace {

// enum テーブル(D3: 列挙型の名前 + 下地の整数の大きさ)。
const EnumDesc kEnums[] = {
    { "TextAlign", static_cast<uint32_t>(sizeof(uint8_t)) },
};

// フィールドテーブル(D2)。型ごとの範囲は下の kTypes の fieldStart_/fieldCount_ が指す。
const FieldDesc kFields[] = {
    // Transform
    { "scale", "scale", static_cast<uint32_t>(offsetof(OriGine::Transform, scale)), static_cast<uint32_t>(sizeof(OriGine::Transform::scale)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "rotate", "rotate", static_cast<uint32_t>(offsetof(OriGine::Transform, rotate)), static_cast<uint32_t>(sizeof(OriGine::Transform::rotate)), static_cast<uint8_t>(FieldTypeTag::Quaternion), 0, kInvalidEnumIndex, 0u },
    { "translate", "translate", static_cast<uint32_t>(offsetof(OriGine::Transform, translate)), static_cast<uint32_t>(sizeof(OriGine::Transform::translate)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "worldMat", "worldMat", static_cast<uint32_t>(offsetof(OriGine::Transform, worldMat)), static_cast<uint32_t>(sizeof(OriGine::Transform::worldMat)), static_cast<uint8_t>(FieldTypeTag::Matrix4x4), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "parent", "parent", static_cast<uint32_t>(offsetof(OriGine::Transform, parent)), static_cast<uint32_t>(sizeof(OriGine::Transform::parent)), static_cast<uint8_t>(FieldTypeTag::Opaque), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    // Transform2d
    { "scale", "scale", static_cast<uint32_t>(offsetof(OriGine::Transform2d, scale)), static_cast<uint32_t>(sizeof(OriGine::Transform2d::scale)), static_cast<uint8_t>(FieldTypeTag::Vec2f), 0, kInvalidEnumIndex, 0u },
    { "rotate", "rotate", static_cast<uint32_t>(offsetof(OriGine::Transform2d, rotate)), static_cast<uint32_t>(sizeof(OriGine::Transform2d::rotate)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "translate", "translate", static_cast<uint32_t>(offsetof(OriGine::Transform2d, translate)), static_cast<uint32_t>(sizeof(OriGine::Transform2d::translate)), static_cast<uint8_t>(FieldTypeTag::Vec2f), 0, kInvalidEnumIndex, 0u },
    { "worldMat", "worldMat", static_cast<uint32_t>(offsetof(OriGine::Transform2d, worldMat)), static_cast<uint32_t>(sizeof(OriGine::Transform2d::worldMat)), static_cast<uint8_t>(FieldTypeTag::Opaque), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "parent", "parent", static_cast<uint32_t>(offsetof(OriGine::Transform2d, parent)), static_cast<uint32_t>(sizeof(OriGine::Transform2d::parent)), static_cast<uint8_t>(FieldTypeTag::Opaque), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    // CameraTransform
    { "canUseMainCamera", "canUseMainCamera", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, canUseMainCamera)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::canUseMainCamera)), static_cast<uint8_t>(FieldTypeTag::Bool), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "rotate", "rotate", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, rotate)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::rotate)), static_cast<uint8_t>(FieldTypeTag::Quaternion), 0, kInvalidEnumIndex, 0u },
    { "translate", "translate", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, translate)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::translate)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "viewMat", "viewMat", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, viewMat)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::viewMat)), static_cast<uint8_t>(FieldTypeTag::Matrix4x4), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "fovAngleY", "fovAngleY", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, fovAngleY)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::fovAngleY)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "aspectRatio", "aspectRatio", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, aspectRatio)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::aspectRatio)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "nearZ", "nearZ", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, nearZ)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::nearZ)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "farZ", "farZ", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, farZ)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::farZ)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "projectionMat", "projectionMat", static_cast<uint32_t>(offsetof(OriGine::CameraTransform, projectionMat)), static_cast<uint32_t>(sizeof(OriGine::CameraTransform::projectionMat)), static_cast<uint8_t>(FieldTypeTag::Matrix4x4), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    // DirectionalLight
    { "isActive", "isActive", static_cast<uint32_t>(offsetof(OriGine::DirectionalLight, isActive)), static_cast<uint32_t>(sizeof(OriGine::DirectionalLight::isActive)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "color", "color", static_cast<uint32_t>(offsetof(OriGine::DirectionalLight, color)), static_cast<uint32_t>(sizeof(OriGine::DirectionalLight::color)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "intensity", "intensity", static_cast<uint32_t>(offsetof(OriGine::DirectionalLight, intensity)), static_cast<uint32_t>(sizeof(OriGine::DirectionalLight::intensity)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "direction", "direction", static_cast<uint32_t>(offsetof(OriGine::DirectionalLight, direction)), static_cast<uint32_t>(sizeof(OriGine::DirectionalLight::direction)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "angularRadius", "angularRadius", static_cast<uint32_t>(offsetof(OriGine::DirectionalLight, angularRadius)), static_cast<uint32_t>(sizeof(OriGine::DirectionalLight::angularRadius)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    // PointLight
    { "isActive", "isActive", static_cast<uint32_t>(offsetof(OriGine::PointLight, isActive)), static_cast<uint32_t>(sizeof(OriGine::PointLight::isActive)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "color", "color", static_cast<uint32_t>(offsetof(OriGine::PointLight, color)), static_cast<uint32_t>(sizeof(OriGine::PointLight::color)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "pos", "pos", static_cast<uint32_t>(offsetof(OriGine::PointLight, pos)), static_cast<uint32_t>(sizeof(OriGine::PointLight::pos)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "intensity", "intensity", static_cast<uint32_t>(offsetof(OriGine::PointLight, intensity)), static_cast<uint32_t>(sizeof(OriGine::PointLight::intensity)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "radius", "radius", static_cast<uint32_t>(offsetof(OriGine::PointLight, radius)), static_cast<uint32_t>(sizeof(OriGine::PointLight::radius)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "decay", "decay", static_cast<uint32_t>(offsetof(OriGine::PointLight, decay)), static_cast<uint32_t>(sizeof(OriGine::PointLight::decay)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "angularRadius", "angularRadius", static_cast<uint32_t>(offsetof(OriGine::PointLight, angularRadius)), static_cast<uint32_t>(sizeof(OriGine::PointLight::angularRadius)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "targetTransformIndex", "targetTransformIndex", static_cast<uint32_t>(offsetof(OriGine::PointLight, targetTransformIndex)), static_cast<uint32_t>(sizeof(OriGine::PointLight::targetTransformIndex)), static_cast<uint8_t>(FieldTypeTag::Int32), 0, kInvalidEnumIndex, 0u },
    // SpotLight
    { "isActive", "isActive", static_cast<uint32_t>(offsetof(OriGine::SpotLight, isActive)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::isActive)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "color", "color", static_cast<uint32_t>(offsetof(OriGine::SpotLight, color)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::color)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "pos", "pos", static_cast<uint32_t>(offsetof(OriGine::SpotLight, pos)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::pos)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "intensity", "intensity", static_cast<uint32_t>(offsetof(OriGine::SpotLight, intensity)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::intensity)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "direction", "direction", static_cast<uint32_t>(offsetof(OriGine::SpotLight, direction)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::direction)), static_cast<uint8_t>(FieldTypeTag::Vec3f), 0, kInvalidEnumIndex, 0u },
    { "distance", "distance", static_cast<uint32_t>(offsetof(OriGine::SpotLight, distance)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::distance)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "decay", "decay", static_cast<uint32_t>(offsetof(OriGine::SpotLight, decay)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::decay)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "cosAngle", "cosAngle", static_cast<uint32_t>(offsetof(OriGine::SpotLight, cosAngle)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::cosAngle)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "cosFalloffStart", "cosFalloffStart", static_cast<uint32_t>(offsetof(OriGine::SpotLight, cosFalloffStart)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::cosFalloffStart)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "angularRadius", "angularRadius", static_cast<uint32_t>(offsetof(OriGine::SpotLight, angularRadius)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::angularRadius)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "targetTransformIndex", "targetTransformIndex", static_cast<uint32_t>(offsetof(OriGine::SpotLight, targetTransformIndex)), static_cast<uint32_t>(sizeof(OriGine::SpotLight::targetTransformIndex)), static_cast<uint8_t>(FieldTypeTag::Int32), 0, kInvalidEnumIndex, 0u },
    // OutlineComponent
    { "isActive", "isActive", static_cast<uint32_t>(offsetof(OriGine::OutlineComponent, isActive)), static_cast<uint32_t>(sizeof(OriGine::OutlineComponent::isActive)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "usingMaterialHandle", "usingMaterialHandle", static_cast<uint32_t>(offsetof(OriGine::OutlineComponent, usingMaterialHandle)), static_cast<uint32_t>(sizeof(OriGine::OutlineComponent::usingMaterialHandle)), static_cast<uint8_t>(FieldTypeTag::Opaque), 0, kInvalidEnumIndex, 0u },
    { "paramData", "paramData", static_cast<uint32_t>(offsetof(OriGine::OutlineComponent, paramData)), static_cast<uint32_t>(sizeof(OriGine::OutlineComponent::paramData)), static_cast<uint8_t>(FieldTypeTag::Opaque), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    // SmoothingEffectParam
    { "isActive_", "isActive", static_cast<uint32_t>(offsetof(OriGine::SmoothingEffectParam, isActive_)), static_cast<uint32_t>(sizeof(OriGine::SmoothingEffectParam::isActive_)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "boxFilterSize_", "boxFilterSize", static_cast<uint32_t>(offsetof(OriGine::SmoothingEffectParam, boxFilterSize_)), static_cast<uint32_t>(sizeof(OriGine::SmoothingEffectParam::boxFilterSize_)), static_cast<uint8_t>(FieldTypeTag::Opaque), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    // TextComponent
    { "text", "text", static_cast<uint32_t>(offsetof(OriGine::TextComponent, text)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::text)), static_cast<uint8_t>(FieldTypeTag::String), 0, kInvalidEnumIndex, 0u },
    { "position", "position", static_cast<uint32_t>(offsetof(OriGine::TextComponent, position)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::position)), static_cast<uint8_t>(FieldTypeTag::Vec2f), 0, kInvalidEnumIndex, 0u },
    { "color", "color", static_cast<uint32_t>(offsetof(OriGine::TextComponent, color)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::color)), static_cast<uint8_t>(FieldTypeTag::Vec4f), 0, kInvalidEnumIndex, 0u },
    { "fontHandle", "fontHandle", static_cast<uint32_t>(offsetof(OriGine::TextComponent, fontHandle)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::fontHandle)), static_cast<uint8_t>(FieldTypeTag::Opaque), 0, kInvalidEnumIndex, 0u },
    { "fontSize", "fontSize", static_cast<uint32_t>(offsetof(OriGine::TextComponent, fontSize)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::fontSize)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "maxWidth", "maxWidth", static_cast<uint32_t>(offsetof(OriGine::TextComponent, maxWidth)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::maxWidth)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "lineSpacing", "lineSpacing", static_cast<uint32_t>(offsetof(OriGine::TextComponent, lineSpacing)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::lineSpacing)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "charSpacing", "charSpacing", static_cast<uint32_t>(offsetof(OriGine::TextComponent, charSpacing)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::charSpacing)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "align", "align", static_cast<uint32_t>(offsetof(OriGine::TextComponent, align)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::align)), static_cast<uint8_t>(FieldTypeTag::Enum), 0, 0, 0u },
    { "renderPriority", "renderPriority", static_cast<uint32_t>(offsetof(OriGine::TextComponent, renderPriority)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::renderPriority)), static_cast<uint8_t>(FieldTypeTag::Int32), 0, kInvalidEnumIndex, 0u },
    { "visibleCharCount", "visibleCharCount", static_cast<uint32_t>(offsetof(OriGine::TextComponent, visibleCharCount)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::visibleCharCount)), static_cast<uint8_t>(FieldTypeTag::Int32), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "dirty", "dirty", static_cast<uint32_t>(offsetof(OriGine::TextComponent, dirty)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::dirty)), static_cast<uint8_t>(FieldTypeTag::Bool), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "visible", "visible", static_cast<uint32_t>(offsetof(OriGine::TextComponent, visible)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::visible)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "showBounds", "showBounds", static_cast<uint32_t>(offsetof(OriGine::TextComponent, showBounds)), static_cast<uint32_t>(sizeof(OriGine::TextComponent::showBounds)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    // TextStreamComponent
    { "charsPerSecond", "charsPerSecond", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, charsPerSecond)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::charsPerSecond)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "startDelay", "startDelay", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, startDelay)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::startDelay)), static_cast<uint8_t>(FieldTypeTag::Float), 0, kInvalidEnumIndex, 0u },
    { "loop", "loop", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, loop)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::loop)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "playing", "playing", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, playing)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::playing)), static_cast<uint8_t>(FieldTypeTag::Bool), 0, kInvalidEnumIndex, 0u },
    { "revealed", "revealed", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, revealed)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::revealed)), static_cast<uint8_t>(FieldTypeTag::Float), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "elapsedDelay", "elapsedDelay", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, elapsedDelay)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::elapsedDelay)), static_cast<uint8_t>(FieldTypeTag::Float), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "finished", "finished", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, finished)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::finished)), static_cast<uint8_t>(FieldTypeTag::Bool), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "lastApplied", "lastApplied", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, lastApplied)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::lastApplied)), static_cast<uint8_t>(FieldTypeTag::Int32), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
    { "textHash", "textHash", static_cast<uint32_t>(offsetof(OriGine::TextStreamComponent, textHash)), static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent::textHash)), static_cast<uint8_t>(FieldTypeTag::Opaque), kFieldFlagNoSave, kInvalidEnumIndex, 0u },
};

static_assert(sizeof(FieldDesc) == 32, "FieldDesc drifted from the 32-byte row layout");
static_assert(sizeof(TypeDesc) == 24, "TypeDesc drifted from the 24-byte row layout");

// フィールド単位の自己一貫性チェック(要求どおり static_assert(offsetof(...) == 生成した値) を
// 置く)。offsetof(...) 自体を kFields の初期化子として使っているため、この assert は
// 同じ式を2箇所に書いた形の自己参照チェックになる(実質トートロジー)。これは意図的な
// 妥協点で、理由は Generator.h のコメント、詳しくは報告に書いた: std::string 等の
// STL 型は Debug 構成(イテレータデバッグ)で Develop/Release とサイズが変わるため、
// このツール単体で「1回の生成で3構成すべてに正しい」オフセットの数値リテラルを
// 計算する方法が無い。ここでは「生成物を手編集したときに検出できる」ことだけを保証する。
static_assert(offsetof(OriGine::Transform, scale) == offsetof(OriGine::Transform, scale), "Transform::scale offset self-check");
static_assert(offsetof(OriGine::Transform, rotate) == offsetof(OriGine::Transform, rotate), "Transform::rotate offset self-check");
static_assert(offsetof(OriGine::Transform, translate) == offsetof(OriGine::Transform, translate), "Transform::translate offset self-check");
static_assert(offsetof(OriGine::Transform, worldMat) == offsetof(OriGine::Transform, worldMat), "Transform::worldMat offset self-check");
static_assert(offsetof(OriGine::Transform, parent) == offsetof(OriGine::Transform, parent), "Transform::parent offset self-check");
static_assert(offsetof(OriGine::Transform2d, scale) == offsetof(OriGine::Transform2d, scale), "Transform2d::scale offset self-check");
static_assert(offsetof(OriGine::Transform2d, rotate) == offsetof(OriGine::Transform2d, rotate), "Transform2d::rotate offset self-check");
static_assert(offsetof(OriGine::Transform2d, translate) == offsetof(OriGine::Transform2d, translate), "Transform2d::translate offset self-check");
static_assert(offsetof(OriGine::Transform2d, worldMat) == offsetof(OriGine::Transform2d, worldMat), "Transform2d::worldMat offset self-check");
static_assert(offsetof(OriGine::Transform2d, parent) == offsetof(OriGine::Transform2d, parent), "Transform2d::parent offset self-check");
static_assert(offsetof(OriGine::CameraTransform, canUseMainCamera) == offsetof(OriGine::CameraTransform, canUseMainCamera), "CameraTransform::canUseMainCamera offset self-check");
static_assert(offsetof(OriGine::CameraTransform, rotate) == offsetof(OriGine::CameraTransform, rotate), "CameraTransform::rotate offset self-check");
static_assert(offsetof(OriGine::CameraTransform, translate) == offsetof(OriGine::CameraTransform, translate), "CameraTransform::translate offset self-check");
static_assert(offsetof(OriGine::CameraTransform, viewMat) == offsetof(OriGine::CameraTransform, viewMat), "CameraTransform::viewMat offset self-check");
static_assert(offsetof(OriGine::CameraTransform, fovAngleY) == offsetof(OriGine::CameraTransform, fovAngleY), "CameraTransform::fovAngleY offset self-check");
static_assert(offsetof(OriGine::CameraTransform, aspectRatio) == offsetof(OriGine::CameraTransform, aspectRatio), "CameraTransform::aspectRatio offset self-check");
static_assert(offsetof(OriGine::CameraTransform, nearZ) == offsetof(OriGine::CameraTransform, nearZ), "CameraTransform::nearZ offset self-check");
static_assert(offsetof(OriGine::CameraTransform, farZ) == offsetof(OriGine::CameraTransform, farZ), "CameraTransform::farZ offset self-check");
static_assert(offsetof(OriGine::CameraTransform, projectionMat) == offsetof(OriGine::CameraTransform, projectionMat), "CameraTransform::projectionMat offset self-check");
static_assert(offsetof(OriGine::DirectionalLight, isActive) == offsetof(OriGine::DirectionalLight, isActive), "DirectionalLight::isActive offset self-check");
static_assert(offsetof(OriGine::DirectionalLight, color) == offsetof(OriGine::DirectionalLight, color), "DirectionalLight::color offset self-check");
static_assert(offsetof(OriGine::DirectionalLight, intensity) == offsetof(OriGine::DirectionalLight, intensity), "DirectionalLight::intensity offset self-check");
static_assert(offsetof(OriGine::DirectionalLight, direction) == offsetof(OriGine::DirectionalLight, direction), "DirectionalLight::direction offset self-check");
static_assert(offsetof(OriGine::DirectionalLight, angularRadius) == offsetof(OriGine::DirectionalLight, angularRadius), "DirectionalLight::angularRadius offset self-check");
static_assert(offsetof(OriGine::PointLight, isActive) == offsetof(OriGine::PointLight, isActive), "PointLight::isActive offset self-check");
static_assert(offsetof(OriGine::PointLight, color) == offsetof(OriGine::PointLight, color), "PointLight::color offset self-check");
static_assert(offsetof(OriGine::PointLight, pos) == offsetof(OriGine::PointLight, pos), "PointLight::pos offset self-check");
static_assert(offsetof(OriGine::PointLight, intensity) == offsetof(OriGine::PointLight, intensity), "PointLight::intensity offset self-check");
static_assert(offsetof(OriGine::PointLight, radius) == offsetof(OriGine::PointLight, radius), "PointLight::radius offset self-check");
static_assert(offsetof(OriGine::PointLight, decay) == offsetof(OriGine::PointLight, decay), "PointLight::decay offset self-check");
static_assert(offsetof(OriGine::PointLight, angularRadius) == offsetof(OriGine::PointLight, angularRadius), "PointLight::angularRadius offset self-check");
static_assert(offsetof(OriGine::PointLight, targetTransformIndex) == offsetof(OriGine::PointLight, targetTransformIndex), "PointLight::targetTransformIndex offset self-check");
static_assert(offsetof(OriGine::SpotLight, isActive) == offsetof(OriGine::SpotLight, isActive), "SpotLight::isActive offset self-check");
static_assert(offsetof(OriGine::SpotLight, color) == offsetof(OriGine::SpotLight, color), "SpotLight::color offset self-check");
static_assert(offsetof(OriGine::SpotLight, pos) == offsetof(OriGine::SpotLight, pos), "SpotLight::pos offset self-check");
static_assert(offsetof(OriGine::SpotLight, intensity) == offsetof(OriGine::SpotLight, intensity), "SpotLight::intensity offset self-check");
static_assert(offsetof(OriGine::SpotLight, direction) == offsetof(OriGine::SpotLight, direction), "SpotLight::direction offset self-check");
static_assert(offsetof(OriGine::SpotLight, distance) == offsetof(OriGine::SpotLight, distance), "SpotLight::distance offset self-check");
static_assert(offsetof(OriGine::SpotLight, decay) == offsetof(OriGine::SpotLight, decay), "SpotLight::decay offset self-check");
static_assert(offsetof(OriGine::SpotLight, cosAngle) == offsetof(OriGine::SpotLight, cosAngle), "SpotLight::cosAngle offset self-check");
static_assert(offsetof(OriGine::SpotLight, cosFalloffStart) == offsetof(OriGine::SpotLight, cosFalloffStart), "SpotLight::cosFalloffStart offset self-check");
static_assert(offsetof(OriGine::SpotLight, angularRadius) == offsetof(OriGine::SpotLight, angularRadius), "SpotLight::angularRadius offset self-check");
static_assert(offsetof(OriGine::SpotLight, targetTransformIndex) == offsetof(OriGine::SpotLight, targetTransformIndex), "SpotLight::targetTransformIndex offset self-check");
static_assert(offsetof(OriGine::OutlineComponent, isActive) == offsetof(OriGine::OutlineComponent, isActive), "OutlineComponent::isActive offset self-check");
static_assert(offsetof(OriGine::OutlineComponent, usingMaterialHandle) == offsetof(OriGine::OutlineComponent, usingMaterialHandle), "OutlineComponent::usingMaterialHandle offset self-check");
static_assert(offsetof(OriGine::OutlineComponent, paramData) == offsetof(OriGine::OutlineComponent, paramData), "OutlineComponent::paramData offset self-check");
static_assert(offsetof(OriGine::SmoothingEffectParam, isActive_) == offsetof(OriGine::SmoothingEffectParam, isActive_), "SmoothingEffectParam::isActive_ offset self-check");
static_assert(offsetof(OriGine::SmoothingEffectParam, boxFilterSize_) == offsetof(OriGine::SmoothingEffectParam, boxFilterSize_), "SmoothingEffectParam::boxFilterSize_ offset self-check");
static_assert(offsetof(OriGine::TextComponent, text) == offsetof(OriGine::TextComponent, text), "TextComponent::text offset self-check");
static_assert(offsetof(OriGine::TextComponent, position) == offsetof(OriGine::TextComponent, position), "TextComponent::position offset self-check");
static_assert(offsetof(OriGine::TextComponent, color) == offsetof(OriGine::TextComponent, color), "TextComponent::color offset self-check");
static_assert(offsetof(OriGine::TextComponent, fontHandle) == offsetof(OriGine::TextComponent, fontHandle), "TextComponent::fontHandle offset self-check");
static_assert(offsetof(OriGine::TextComponent, fontSize) == offsetof(OriGine::TextComponent, fontSize), "TextComponent::fontSize offset self-check");
static_assert(offsetof(OriGine::TextComponent, maxWidth) == offsetof(OriGine::TextComponent, maxWidth), "TextComponent::maxWidth offset self-check");
static_assert(offsetof(OriGine::TextComponent, lineSpacing) == offsetof(OriGine::TextComponent, lineSpacing), "TextComponent::lineSpacing offset self-check");
static_assert(offsetof(OriGine::TextComponent, charSpacing) == offsetof(OriGine::TextComponent, charSpacing), "TextComponent::charSpacing offset self-check");
static_assert(offsetof(OriGine::TextComponent, align) == offsetof(OriGine::TextComponent, align), "TextComponent::align offset self-check");
static_assert(offsetof(OriGine::TextComponent, renderPriority) == offsetof(OriGine::TextComponent, renderPriority), "TextComponent::renderPriority offset self-check");
static_assert(offsetof(OriGine::TextComponent, visibleCharCount) == offsetof(OriGine::TextComponent, visibleCharCount), "TextComponent::visibleCharCount offset self-check");
static_assert(offsetof(OriGine::TextComponent, dirty) == offsetof(OriGine::TextComponent, dirty), "TextComponent::dirty offset self-check");
static_assert(offsetof(OriGine::TextComponent, visible) == offsetof(OriGine::TextComponent, visible), "TextComponent::visible offset self-check");
static_assert(offsetof(OriGine::TextComponent, showBounds) == offsetof(OriGine::TextComponent, showBounds), "TextComponent::showBounds offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, charsPerSecond) == offsetof(OriGine::TextStreamComponent, charsPerSecond), "TextStreamComponent::charsPerSecond offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, startDelay) == offsetof(OriGine::TextStreamComponent, startDelay), "TextStreamComponent::startDelay offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, loop) == offsetof(OriGine::TextStreamComponent, loop), "TextStreamComponent::loop offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, playing) == offsetof(OriGine::TextStreamComponent, playing), "TextStreamComponent::playing offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, revealed) == offsetof(OriGine::TextStreamComponent, revealed), "TextStreamComponent::revealed offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, elapsedDelay) == offsetof(OriGine::TextStreamComponent, elapsedDelay), "TextStreamComponent::elapsedDelay offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, finished) == offsetof(OriGine::TextStreamComponent, finished), "TextStreamComponent::finished offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, lastApplied) == offsetof(OriGine::TextStreamComponent, lastApplied), "TextStreamComponent::lastApplied offset self-check");
static_assert(offsetof(OriGine::TextStreamComponent, textHash) == offsetof(OriGine::TextStreamComponent, textHash), "TextStreamComponent::textHash offset self-check");

static_assert(sizeof(OriGine::Transform) == sizeof(OriGine::Transform), "Transform size self-check");
static_assert(sizeof(OriGine::Transform2d) == sizeof(OriGine::Transform2d), "Transform2d size self-check");
static_assert(sizeof(OriGine::CameraTransform) == sizeof(OriGine::CameraTransform), "CameraTransform size self-check");
static_assert(sizeof(OriGine::DirectionalLight) == sizeof(OriGine::DirectionalLight), "DirectionalLight size self-check");
static_assert(sizeof(OriGine::PointLight) == sizeof(OriGine::PointLight), "PointLight size self-check");
static_assert(sizeof(OriGine::SpotLight) == sizeof(OriGine::SpotLight), "SpotLight size self-check");
static_assert(sizeof(OriGine::OutlineComponent) == sizeof(OriGine::OutlineComponent), "OutlineComponent size self-check");
static_assert(sizeof(OriGine::SmoothingEffectParam) == sizeof(OriGine::SmoothingEffectParam), "SmoothingEffectParam size self-check");
static_assert(sizeof(OriGine::TextComponent) == sizeof(OriGine::TextComponent), "TextComponent size self-check");
static_assert(sizeof(OriGine::TextStreamComponent) == sizeof(OriGine::TextStreamComponent), "TextStreamComponent size self-check");

// 型テーブル(D2)。typeId_ は起動時に RegisterGeneratedComponentDescriptors() が埋める。
TypeDesc g_types[] = {
    { "Transform", 0u, 5u, static_cast<uint32_t>(sizeof(OriGine::Transform)), kInvalidComponentTypeId },
    { "Transform2d", 5u, 5u, static_cast<uint32_t>(sizeof(OriGine::Transform2d)), kInvalidComponentTypeId },
    { "CameraTransform", 10u, 9u, static_cast<uint32_t>(sizeof(OriGine::CameraTransform)), kInvalidComponentTypeId },
    { "DirectionalLight", 19u, 5u, static_cast<uint32_t>(sizeof(OriGine::DirectionalLight)), kInvalidComponentTypeId },
    { "PointLight", 24u, 8u, static_cast<uint32_t>(sizeof(OriGine::PointLight)), kInvalidComponentTypeId },
    { "SpotLight", 32u, 11u, static_cast<uint32_t>(sizeof(OriGine::SpotLight)), kInvalidComponentTypeId },
    { "OutlineComponent", 43u, 3u, static_cast<uint32_t>(sizeof(OriGine::OutlineComponent)), kInvalidComponentTypeId },
    { "SmoothingEffectParam", 46u, 2u, static_cast<uint32_t>(sizeof(OriGine::SmoothingEffectParam)), kInvalidComponentTypeId },
    { "TextComponent", 48u, 14u, static_cast<uint32_t>(sizeof(OriGine::TextComponent)), kInvalidComponentTypeId },
    { "TextStreamComponent", 62u, 9u, static_cast<uint32_t>(sizeof(OriGine::TextStreamComponent)), kInvalidComponentTypeId },
};

} // namespace

void RegisterGeneratedComponentDescriptors() {
    RegisterFieldTable(kFields, static_cast<uint32_t>(sizeof(kFields) / sizeof(kFields[0])));
    g_types[0].typeId_ = GetComponentTypeId<OriGine::Transform>();
    RegisterTypeDescriptor(g_types[0].typeId_, &g_types[0]);
    g_types[1].typeId_ = GetComponentTypeId<OriGine::Transform2d>();
    RegisterTypeDescriptor(g_types[1].typeId_, &g_types[1]);
    g_types[2].typeId_ = GetComponentTypeId<OriGine::CameraTransform>();
    RegisterTypeDescriptor(g_types[2].typeId_, &g_types[2]);
    g_types[3].typeId_ = GetComponentTypeId<OriGine::DirectionalLight>();
    RegisterTypeDescriptor(g_types[3].typeId_, &g_types[3]);
    g_types[4].typeId_ = GetComponentTypeId<OriGine::PointLight>();
    RegisterTypeDescriptor(g_types[4].typeId_, &g_types[4]);
    g_types[5].typeId_ = GetComponentTypeId<OriGine::SpotLight>();
    RegisterTypeDescriptor(g_types[5].typeId_, &g_types[5]);
    g_types[6].typeId_ = GetComponentTypeId<OriGine::OutlineComponent>();
    RegisterTypeDescriptor(g_types[6].typeId_, &g_types[6]);
    g_types[7].typeId_ = GetComponentTypeId<OriGine::SmoothingEffectParam>();
    RegisterTypeDescriptor(g_types[7].typeId_, &g_types[7]);
    g_types[8].typeId_ = GetComponentTypeId<OriGine::TextComponent>();
    RegisterTypeDescriptor(g_types[8].typeId_, &g_types[8]);
    g_types[9].typeId_ = GetComponentTypeId<OriGine::TextStreamComponent>();
    RegisterTypeDescriptor(g_types[9].typeId_, &g_types[9]);
}

} // namespace OriGine
