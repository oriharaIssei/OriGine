#pragma once

#include "component/IComponent.h"

#include <string>
#include <vector>

class EntityReferenceList : public IComponent {
    friend void to_json(nlohmann::json& j, const EntityReferenceList& c);
    friend void from_json(const nlohmann::json& j, EntityReferenceList& c);

public:
    EntityReferenceList();
    ~EntityReferenceList() override;
    void Initialize(GameEntity* _entity) override;
    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

private:
    // first: directory, second: filename
    std::vector<std::pair<std::string, std::string>> entityFileList_;

public:
    const std::vector<std::pair<std::string, std::string>>& getEntityFileList() const {
        return entityFileList_;
    }
};
