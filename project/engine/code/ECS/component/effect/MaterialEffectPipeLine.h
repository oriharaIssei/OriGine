#pragma once
#include "component/IComponent.h"

/// stl
#include <array>
#include <memory>
#include <string>

//// engine
// directX12
#include "directX12/DxDescriptor.h"

enum class MaterialEffectType {
    Dissolve,
    Distortion,
    Gradation,

    Count
};
static const std::array<std::string, static_cast<int32_t>(MaterialEffectType::Count)> materialEffectString{
    "Dissolve",
    "Distortion",
    "Gradation"};

/// <summary>
/// Materialに設定したエフェクトを掛けるためのパイプライン
/// </summary>
class MaterialEffectPipeLine
    : public IComponent {
    friend void to_json(nlohmann::json& j, const MaterialEffectPipeLine& c);
    friend void from_json(const nlohmann::json& j, MaterialEffectPipeLine& c);

public:
    MaterialEffectPipeLine();
    ~MaterialEffectPipeLine() override;
    void Initialize(Entity* _entity) override;
    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    /// <summary>
    /// Effectをかけられるテクスチャを読み込む
    /// </summary>
    /// <param name="_path"></param>
    void LoadBaseTexture(const std::string& _path);
    /// <summary>
    /// EffectをかけるEntityを追加
    /// </summary>
    /// <param name="_type"></param>
    /// <param name="_entityID"></param>
    void AddEffectEntity(MaterialEffectType _type, int32_t _entityID) {
        effectEntityIdList_.push_back(EffectEntityData{_type, _entityID});
    }
    void ClearEffectEntity() { effectEntityIdList_.clear(); }

private:
    struct EffectEntityData {
        MaterialEffectType effectType = MaterialEffectType::Dissolve;
        int32_t entityID              = -1;
    };
    std::vector<EffectEntityData> effectEntityIdList_ = {};

    int32_t priority_ = 0;
    bool isActive_    = true;
    // 結果テクスチャを保持し,BaseTextureを animationさせるMaterialのIndex
    int32_t materialIndex_ = -1;
    // Effectをかける テクスチャ
    int32_t baseTextureId_ = 0;

    std::string baseTexturePath_ = "";

public:
    int32_t GetPriority() const { return priority_; }
    void SetPriority(int32_t priority) { priority_ = priority; }
    bool IsActive() const { return isActive_; }
    void SetActive(bool active) { isActive_ = active; }

    int32_t GetMaterialIndex() const { return materialIndex_; }
    void SetMaterialIndex(int32_t index) { materialIndex_ = index; }

    int32_t GetBaseTextureId() const { return baseTextureId_; }
    const std::vector<EffectEntityData>& GetEffectEntityIdList() const { return effectEntityIdList_; }
};
