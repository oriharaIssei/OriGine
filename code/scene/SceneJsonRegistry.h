#pragma once

/// stl
#include <string>
#include <unordered_map>

/// engine
#include "entity/Entity.h"
#include "scene/Scene.h"

/// externals
#include <nlohmann/json.hpp>

namespace OriGine {

constexpr char kSceneJsonFolder[]      = "scene";
constexpr char kEntityTemplateFolder[] = "entities";
constexpr char kEntityExtension[]      = "ent";

/// <summary>
/// アセットディレクトリに含まれるシーン (.json) やエンティティテンプレート (.ent) の JSON データを管理するレジストリクラス.
/// メモリ上に展開された JSON データを一括で保存・読み込みする機能を提供し、エディタ上での編集結果の永続化などを支える.
/// </summary>
class SceneJsonRegistry {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する.
    /// </summary>
    static SceneJsonRegistry* GetInstance();

    /// <summary>
    /// 指定した名前でシーンの JSON データをレジストリに登録する.
    /// </summary>
    /// <param name="_sceneName">シーンの識別名</param>
    /// <param name="_data">登録する JSON データ</param>
    void RegisterSceneJson(const std::string& _sceneName, const nlohmann::json& _data) {
        scenes_[_sceneName] = _data;
    }

    /// <summary>
    /// レジストリから指定したシーンの JSON データを取得する.
    /// </summary>
    /// <param name="_sceneName">シーンの識別名</param>
    /// <returns>JSON データへのポインタ. 存在しない場合は nullptr</returns>
    const nlohmann::json* GetSceneJson(const std::string& _sceneName) const {
        auto it = scenes_.find(_sceneName);
        return (it != scenes_.end()) ? &it->second : nullptr;
    }

    /// <summary>
    /// レジストリに保持されているすべてのシーンデータを、指定したディレクトリ配下のファイルに一括保存する.
    /// </summary>
    /// <param name="_directory">保存先ディレクトリのパス</param>
    /// <returns>保存に成功した場合は true</returns>
    bool SaveAllScene(const std::string& _directory);

    /// <summary>
    /// 指定された実行中の Scene オブジェクトから最新の JSON を生成し、指定ディレクトリに保存する.
    /// </summary>
    /// <param name="_scene">保存対象のシーン</param>
    /// <param name="_directory">保存先のルートディレクトリ</param>
    /// <returns>成功した場合は true</returns>
    bool SaveScene(const Scene* _scene, const std::string& _directory);

    /// <summary>
    /// 指定ディレクトリからすべてのシーン JSON を再帰的に検索し、レジストリにロードする.
    /// </summary>
    /// <param name="_directory">検索対象のディレクトリパス</param>
    /// <returns>ロードに成功した場合は true</returns>
    bool LoadAllScene(const std::string& _directory);

    /// <summary>
    /// 単一のシーンファイルをファイルシステムから読み込む.
    /// </summary>
    /// <param name="_sceneName">ロードするシーンの名前</param>
    /// <param name="_directory">検索対象のディレクトリ</param>
    /// <returns>成功した場合は true</returns>
    bool LoadScene(const std::string& _sceneName, const std::string& _directory);

    /// <summary>
    /// 新規作成用の空のシーン JSON データを生成し、レジストリに登録する.
    /// </summary>
    /// <param name="_sceneName">新規シーンの名前</param>
    void CreateNewScene(const std::string& _sceneName);

    /// <summary>
    /// 指定ディレクトリからすべてのエンティティテンプレート (.ent) を読み込み、レジストリに登録する.
    /// </summary>
    /// <param name="_directory">エンティティテンプレートの格納ディレクトリ</param>
    bool LoadAllEntityTemplates(const std::string& _directory);

    /// <summary>
    /// 指定した型名のエンティティテンプレートを個別にファイルから読み込む.
    /// </summary>
    /// <param name="_directory">ディレクトリ</param>
    /// <param name="_typeName">型名 (ファイル名と一致させる必要がある)</param>
    /// <returns>成功した場合は true</returns>
    bool LoadEntityTemplate(const std::string& _directory, const std::string& _typeName);

    /// <summary>
    /// 指定した型名のエンティティテンプレートをファイルとして書き出す.
    /// </summary>
    /// <param name="_directory">保存先ディレクトリ</param>
    /// <param name="_typeName">型名</param>
    /// <returns>成功した場合は true</returns>
    bool SaveEntityTemplate(const std::string& _directory, const std::string& _typeName);

    /// <summary>
    /// JSON オブジェクトを直接エンティティテンプレートとしてレジストリに登録する.
    /// </summary>
    /// <param name="_typeName">登録する型名</param>
    /// <param name="_json">エンティティの定義 JSON</param>
    void RegisterEntityTemplate(const std::string& _typeName, const nlohmann::json& _json);

    /// <summary>
    /// 実行中の特定のエンティティの状態をシリアライズし、テンプレートとしてレジストリに登録する.
    /// </summary>
    /// <param name="_typeName">登録する型名</param>
    /// <param name="_scene">エンティティが属するシーン</param>
    /// <param name="_entity">対象のエンティティポインタ</param>
    void RegisterEntityTemplateFromEntity(const std::string& _typeName, Scene* _scene, Entity* _entity);

    /// <summary>
    /// 登録済みのエンティティテンプレートを取得する.
    /// </summary>
    /// <param name="_typeName">取得したい型名</param>
    /// <returns>JSON データへのポインタ. 存在しない場合は nullptr</returns>
    const nlohmann::json* GetEntityTemplate(const std::string& _typeName) const;

private:
    /// <summary>
    /// ロード済みのシーンデータ (シーン名 -> JSON)
    /// </summary>
    std::unordered_map<std::string, nlohmann::json> scenes_;

    /// <summary>
    /// ロード済みのエンティティテンプレート (型名 -> JSON)
    /// </summary>
    std::unordered_map<std::string, nlohmann::json> entityTemplates_;
};

} // namespace OriGine
