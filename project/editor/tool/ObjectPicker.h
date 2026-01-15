#pragma once

/// stl
#include <memory>
#include <vector>

/// engine
#include "scene/Scene.h"
// directX12
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/RwStructuredBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/raytracing/RaytracingScene.h"

/// ECS
// component
#include "component/transform/CameraTransform.h"

/// math
#include "math/Matrix4x4.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

namespace OriGine {

/// <summary>
/// オブジェクトピッカー用シーンデータ
/// </summary>
struct ObjectPickerSceneData {
    Vec3f rayOrigin;
    float cameraNear; // カメラのニアクリップ距離
    Vec3f rayDirection;
    float cameraFar; // カメラのファークリップ距離

    struct ConstantBuffer {
        Vec3f rayOrigin;
        float cameraNear; // カメラのニアクリップ距離
        Vec3f rayDirection;
        float cameraFar; // カメラのファークリップ距離

        ConstantBuffer& operator=(const ObjectPickerSceneData& _data) {
            rayOrigin    = _data.rayOrigin;
            rayDirection = _data.rayDirection;
            cameraNear   = _data.cameraNear;
            cameraFar    = _data.cameraFar;

            return *this;
        }
    };
};

struct PickedObjectIndex {
    int32_t objectIndex = -1; // ピックされたオブジェクトのインデックス (-1なら何もピックされていない)

    struct ConstantBuffer {
        int32_t objectIndex = -1;
        int32_t padding[3]; // ここも合わせる

        ConstantBuffer& operator=(const PickedObjectIndex& _data) {
            objectIndex = _data.objectIndex;
            return *this;
        }
    };
};

/// <summary>
/// クリックした一からオブジェクトを選択するためのツール
/// </summary>
class ObjectPicker {
public:
    ObjectPicker();
    ~ObjectPicker();

    /// <summary>
    /// ツールの初期化
    /// </summary>
    void Initialize();
    /// <summary>
    /// ツールの終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// ツールを有効化する
    /// </summary>
    void Activate(Scene* _scene);
    /// <summary>
    /// ツールを無効化する
    /// </summary>
    void Deactivate();

    /// <summary>
    /// クリック位置からオブジェクトを取得する
    /// </summary>
    /// <param name="_scene"></param>
    /// <param name="_clickPos"></param>
    /// <returns></returns>
    EntityHandle PickedObject(const CameraTransform& _cameraTransform, const Vec2f& _clickPos, const Vec2f& _screenResolution);

private:
    /// <summary>
    /// パイプラインステートオブジェクトの作成
    /// </summary>
    void CreatePSO();

    /// <summary>
    /// レイトレーシングで使用するメッシュを登録する
    /// </summary>
    void DispatchMeshForRaytracing();

    /// <summary>
    /// レイトレーシングシーンの更新
    /// </summary>
    void UpdateRaytracingScene();

private:
    bool isActive_         = false; // ツールが有効かどうか
    Scene* scene_          = nullptr;
    PipelineStateObj* pso_ = nullptr;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

    IConstantBuffer<ObjectPickerSceneData> sceneDataBuffer_;
    RwStructuredBuffer<PickedObjectIndex> pickedObjectBuffer_;

    /// <summary>レイトレーシング用シーン情報の管理オブジェクト</summary>
    std::unique_ptr<RaytracingScene> raytracingScene_ = nullptr;

    std::vector<RaytracingMeshEntry> meshForRaytracing_{}; // レイトレーシング用メッシュのエントリ
    std::vector<EntityHandle> entityHandles_{}; // メッシュエントリに対応するエンティティハンドル
    std::vector<RayTracingInstance> rayTracingInstances_{}; // レイトレーシングインスタンス
    std::unordered_map<int32_t, EntityHandle> indexToEntityMap_{};
};

}
