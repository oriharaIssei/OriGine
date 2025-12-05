#pragma once

#include "component/IComponent.h"

#include <string>
#include <vector>

/// <summary>
/// エンティティ参照リストコンポーネント
/// 他のエンティティファイルへの参照を保持するコンポーネント
/// </summary>
class EntityReferenceList
    : public OriGine::IComponent {
    friend void to_json(nlohmann::json& j, const EntityReferenceList& c);
    friend void from_json(const nlohmann::json& j, EntityReferenceList& c);

public:
    EntityReferenceList();
    ~EntityReferenceList() override;
    void Initialize(Entity* _entity) override;
    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

private:
    // first: directory, second: filename
    std::vector<std::pair<std::string, std::string>> entityFileList_;

public:
    const std::vector<std::pair<std::string, std::string>>& GetEntityFileList() const {
        return entityFileList_;
    }
};
