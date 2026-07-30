#pragma once

/// stl
// memory
#include <functional>
#include <memory>
// basic class
#include <string>
// container
#include <vector>

namespace OriGine {
/// 前方宣言
/// engine
// assetes
struct Model;
struct ModelMeshData;
struct TexturedMaterial;

/// <summary>
/// Model インスタンスの生成を担当するクラス.
///
/// モデルファイルそのもの（メッシュ・ノード階層・スケルトン・既定マテリアル）の
/// 読み込みとキャッシュは ModelAssetManager が担当する.
/// このクラスが受け持つのは「共有アセットを参照する Model インスタンスを組み立てる」
/// 部分のみで、インスタンス固有のマテリアルバッファや Transform をここで用意する.
/// </summary>
class ModelManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する.
    /// </summary>
    /// <returns>インスタンスのポインタ</returns>
    static ModelManager* GetInstance();

    /// <summary>
    /// モデルアセットをロード（またはキャッシュから取得）し、Model インスタンスを作成する.
    /// </summary>
    /// <param name="_directoryPath">ファイルが存在するディレクトリの相対パス</param>
    /// <param name="_filename">モデルのファイル名（拡張子含む）</param>
    /// <param name="_callBack">生成完了時に実行されるコールバック</param>
    /// <returns>作成された Model クラスの共有ポインタ. 失敗時は nullptr</returns>
    std::shared_ptr<Model> Create(
        const std::string& _directoryPath,
        const std::string& _filename,
        std::function<void(Model*)> _callBack = nullptr);

    /// <summary>
    /// マネージャの初期化を行う.
    /// </summary>
    void Initialize();

    /// <summary>
    /// マネージャの終了処理を行う.
    /// </summary>
    void Finalize();

    /// <summary>
    /// モデルデータを取得する. 未ロードの場合はここでロードを行う.
    /// </summary>
    /// <param name="_directoryPath">ディレクトリパス</param>
    /// <param name="_filename">ファイル名</param>
    /// <returns>モデルデータポインタ. 取得できない場合は nullptr</returns>
    ModelMeshData* GetModelMeshData(const std::string& _directoryPath, const std::string& _filename);

    /// <summary>
    /// 指定されたモデルデータに紐付くデフォルトマテリアルリストを取得する.
    /// </summary>
    /// <param name="_key">モデルデータポインタ</param>
    /// <returns>マテリアルのベクトルへの参照</returns>
    const std::vector<TexturedMaterial>& GetDefaultMaterials(ModelMeshData* _key) const;

    /// <summary>
    /// 指定されたファイルパスのモデルに紐付くデフォルトマテリアルリストを取得する.
    /// </summary>
    /// <param name="_directoryPath">ディレクトリパス</param>
    /// <param name="_filename">ファイル名</param>
    /// <returns>マテリアルのベクトルへの参照</returns>
    const std::vector<TexturedMaterial>& GetDefaultMaterials(const std::string& _directoryPath, const std::string& _filename);

private:
    /// <summary>
    /// ディレクトリとファイル名から、AssetManager に渡すアセットパスを組み立てる.
    /// </summary>
    static std::string MakeAssetPath(const std::string& _directoryPath, const std::string& _filename);
};

} // namespace OriGine
