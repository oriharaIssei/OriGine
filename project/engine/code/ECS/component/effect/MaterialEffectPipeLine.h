#pragma once
#include "component/IComponent.h"

/// stl
#include <memory>

//// engine
// directX12
#include "directX12/DxDescriptor.h"

enum class MaterialEffectType {
    None,
    Dissolve,
    Distortion,

    Count
};

class MaterialEffectPipeLine
    : public IComponent {
    friend void to_json(nlohmann::json& j, const MaterialEffectPipeLine& c);
    friend void from_json(const nlohmann::json& j, MaterialEffectPipeLine& c);

public:
    MaterialEffectPipeLine();
    ~MaterialEffectPipeLine() override;
    void Initialize(GameEntity* _entity) override;
    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    void LoadBaseTexture(const std::string& _path);
    void AddEffectEntity(MaterialEffectType _type, int32_t _entityID) {
        effectEntityIdList_.push_back(EffectEntityData{_type, _entityID});
    }
    void ClearEffectEntity() { effectEntityIdList_.clear(); }

private:
    void CreateEffectedTextureResourceByBaseTexture();

private:
    struct EffectEntityData {
        MaterialEffectType effectType = MaterialEffectType::None;
        int32_t entityID              = -1;
    };
    std::vector<EffectEntityData> effectEntityIdList_ = {};

    bool isActive_ = true;

    std::string baseTexturePath_                         = "";
    int32_t baseTextureId_                               = -1;
    DxResource effectedTextureResource_                  = {};
    std::shared_ptr<DxSrvDescriptor> effectedTextureSrv_ = nullptr;

    Vec2f textureSize_ = {0.f, 0.f};

public:
    bool isActive() const { return isActive_; }
    void setActive(bool active) { isActive_ = active; }

    std::shared_ptr<DxSrvDescriptor> getEffectedTextureSrv() const { return effectedTextureSrv_; }
    void setEffectedTextureSrv(std::shared_ptr<DxSrvDescriptor> srv) { effectedTextureSrv_ = srv; }
    DxResource& getEffectedTextureResource() { return effectedTextureResource_; }

    const Vec2f& getTextureSize() const { return textureSize_; }

    int32_t getBaseTextureId() const { return baseTextureId_; }
    const std::vector<EffectEntityData>& getEffectEntityIdList() const { return effectEntityIdList_; }
};
