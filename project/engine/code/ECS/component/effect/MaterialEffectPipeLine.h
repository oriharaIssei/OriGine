#pragma once
#include "component/IComponent.h"

enum class MaterialEffectType {
    None,
    Dissolve,
    Distortion,

    Count
};

class MaterialEffectPipeLine
    : public IComponent {
public:
    MaterialEffectPipeLine();
    ~MaterialEffectPipeLine() override;
    void Initialize(GameEntity* _entity) override;
    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    void LoadBaseTexture(const std::string& _path);
    void AddEffectEntity(MaterialEffectType _type, int32_t _entityID) {
        effectEntityDataList_.push_back(EffectEntityData{_type, _entityID});
    }
    void ClearEffectEntity() { effectEntityDataList_.clear(); }

private:
    struct EffectEntityData {
        MaterialEffectType effectType = MaterialEffectType::None;
        int32_t entityID              = -1;
    };
    std::vector<EffectEntityData> effectEntityDataList_;

    std::string baseTexturePath_ = "";
    int32_t baseTextureId_       = -1;

public:
    int32_t getBaseTextureId() const { return baseTextureId_; }
    const std::vector<EffectEntityData>& getEffectEntityDataList() const { return effectEntityDataList_; }
};
