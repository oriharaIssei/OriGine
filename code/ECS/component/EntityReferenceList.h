#pragma once

#include "component/IComponent.h"

/// stl
#include <string>
#include <vector>

namespace OriGine {

/// <summary>
/// エンティティ参照リストコンポーネント
/// 他のエンティティファイルへの参照を保持するコンポーネント
/// </summary>
class EntityReferenceList : public IComponent {
    friend void to_json(nlohmann::json& j, const EntityReferenceList& c);
    friend void from_json(const nlohmann::json& j, EntityReferenceList& c);

public:
    EntityReferenceList();
    ~EntityReferenceList() override;
    /// <summary>
    /// 初期化処理(現状は特別な処理なし)
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _entity) override;
    /// <summary>
    /// エディタ上での編集UIを描画する処理
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;
    /// <summary>
    /// 終了処理(現状は特別な処理なし)
    /// </summary>
    void Finalize() override;

private:
    // first: directory, second: filename
    std::vector<std::pair<std::string, std::string>> entityFileList_; // 参照先エンティティファイルの(ディレクトリ, ファイル名)一覧

public:
    const std::vector<std::pair<std::string, std::string>>& GetEntityFileList() const {
        return entityFileList_;
    }
};

} // namespace OriGine
