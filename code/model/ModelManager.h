#pragma once

/// stl
// memory
#include <functional>
#include <memory>
// contiainer
#include <array>
#include <unordered_map>
// basic class
#include <string>

// dx12Object
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

// math
#include "Matrix4x4.h"

namespace OriGine {
/// 前方宣言
/// engine
// assetes
struct Model;
struct ModelNode;
struct ModelMeshData;
struct TexturedMaterial;

/// <summary>
/// モデルリソースのロードと一元管理を行うシングルトンクラス.
/// 同じモデルファイルが複数回要求された場合、メモリ上のデータを共有するよう制御する.
/// </summary>
class ModelManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する.
    /// </summary>
    /// <returns>インスタンスのポインタ</returns>
    static ModelManager* GetInstance();

    /// <summary>
    /// モデルファイルをロード（またはキャッシュから取得）し、Model インスタンスを作成する.
    /// </summary>
    /// <param name="_directoryPath">ファイルが存在するディレクトリの相対パス</param>
    /// <param name="_filename">モデルのファイル名（拡張子含む）</param>
    /// <param name="_callBack">ロード完了時に実行されるコールバック</param>
    /// <returns>作成された Model クラスの共有ポインタ</returns>
    ::std::shared_ptr<Model> Create(
        const ::std::string& _directoryPath,
        const ::std::string& _filename,
        ::std::function<void(Model*)> _callBack = nullptr);

    /// <summary>
    /// マネージャの初期化を行う. 内部で使用するコマンドリストや PSO を準備する.
    /// </summary>
    void Initialize();

    /// <summary>
    /// マネージャの終了処理を行う. キャッシュされている全モデルリソースを解放する.
    /// </summary>
    void Finalize();

private:
    /// <summary>ブレンドモードごとの PSO 配列</summary>
    ::std::array<PipelineStateObj*, kBlendNum> texturePso_;
    /// <summary>モデルデータ転送用のコマンドオブジェクト</summary>
    ::std::unique_ptr<DxCommand> dxCommand_;

    /// <summary>（将来的な拡張用）FOV関連の行列</summary>
    ::std::unique_ptr<Matrix4x4> fovMa_;

    /// <summary>
    /// バックグラウンドでのモデルロードタスクを表す内部構造体.
    /// </summary>
    struct LoadTask {
        ::std::string directory;
        ::std::string fileName;
        ::std::shared_ptr<Model> model = nullptr;

        ::std::function<void(Model*)> callBack = nullptr;

        /// <summary>
        /// タスクの実行（ロード処理）を行う.
        /// </summary>
        void Update();
    };

private:
    /// <summary>ファイル名をキーとしたモデルリソース（静的データ）のキャッシュ</summary>
    ::std::unordered_map<::std::string, ::std::unique_ptr<ModelMeshData>> modelLibrary_;
    /// <summary>モデルデータごとのデフォルトマテリアル情報の保持</summary>
    ::std::unordered_map<ModelMeshData*, ::std::vector<TexturedMaterial>> defaultMaterials_;

public:
    /// <summary>
    /// 指定されたモデルデータに対してデフォルトのマテリアル情報を登録する.
    /// 通常、モデルロード時にエンジンの内部処理として呼ばれる.
    /// </summary>
    /// <param name="_key">対象のモデルデータ</param>
    /// <param name="_material">登録するマテリアル情報</param>
    void pushBackDefaultMaterial(ModelMeshData* _key, TexturedMaterial _material);

    /// <summary>
    /// ロード済みのモデルデータを取得する.
    /// </summary>
    /// <param name="_directoryPath">ディレクトリパス</param>
    /// <param name="_filename">ファイル名</param>
    /// <returns>モデルデータポインタ. 未ロードの場合は nullptr を返すことがある.</returns>
    ModelMeshData* GetModelMeshData(const ::std::string& _directoryPath, const ::std::string& _filename);

    /// <summary>
    /// 指定されたモデルデータに紐付くデフォルトマテリアルリストを取得する.
    /// </summary>
    /// <param name="_key">モデルデータポインタ</param>
    /// <returns>マテリアルのベクトルへの参照</returns>
    const ::std::vector<TexturedMaterial>& GetDefaultMaterials(ModelMeshData* _key) const;

    /// <summary>
    /// 指定されたファイルパスのモデルに紐付くデフォルトマテリアルリストを取得する.
    /// </summary>
    /// <param name="_directoryPath">ディレクトリパス</param>
    /// <param name="_filename">ファイル名</param>
    /// <returns>マテリアルのベクトルへの参照</returns>
    const ::std::vector<TexturedMaterial>& GetDefaultMaterials(const ::std::string& _directoryPath, const ::std::string& _filename) const;
};

} // namespace OriGine
