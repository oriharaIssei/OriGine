#pragma once

/// stl
#include <memory>

/// engine
#include "component/transform/CameraTransform.h"
#include "directX12/buffer/IConstantBuffer.h"

#include "camera/debugCamera/DebugCamera.h"

namespace OriGine {

/// <summary>
/// カメラを管理するクラス.
/// Scene名ごとに CameraTransform を保持する.
/// </summary>
class CameraManager {
public:
    static CameraManager* GetInstance();

    void Initialize();
    void Finalize();

    /// <summary>
    /// Scene ごとのカメラ登録
    /// </summary>
    /// <param name="_sceneName"></param>
    /// <returns></returns>
    bool RegisterSceneCamera(const std::string& _sceneName);
    /// <summary>
    /// Scene ごとのカメラ登録解除
    /// </summary>
    /// <param name="_sceneName"></param>
    /// <returns></returns>
    bool UnregisterSceneCamera(const std::string& _sceneName);

    /// <summary>
    /// Scene ごとのカメラ登録
    /// </summary>
    /// <param name="_scene"></param>
    /// <returns></returns>
    bool RegisterSceneCamera(Scene* _scene);
    /// <summary>
    /// Scene ごとのカメラ登録解除
    /// </summary>
    /// <param name="_scene"></param>
    /// <returns></returns>
    bool UnregisterSceneCamera(Scene* _scene);

    /// <summary>
    /// カメラの更新
    /// </summary>
    /// <param name="_sceneName"></param>
    /// <param name="transform"></param>
    /// <returns></returns>
    bool SetTransform(const std::string& _sceneName, const CameraTransform& transform);
    bool SetTransform(Scene* _scene, const CameraTransform& transform);

    /// <summary>
    /// バッファへデータ変換
    /// </summary>
    /// <param name="_sceneName"></param>
    /// <returns></returns>
    bool DataConvertToBuffer(const std::string& _sceneName);
    bool DataConvertToBuffer(Scene* _scene);

    /// <summary>
    /// ルートパラメータにバッファをセット
    /// </summary>
    /// <param name="_sceneName"></param>
    /// <param name="cmdList"></param>
    /// <param name="rootParameterNum"></param>
    /// <returns></returns>
    bool SetBufferForRootParameter(
        const std::string& _sceneName,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
        uint32_t rootParameterNum);

    bool SetBufferForRootParameter(
        Scene* _scene,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
        uint32_t rootParameterNum);

    CameraTransform GetTransform(const std::string& _sceneName)const;
    CameraTransform GetTransform(Scene* _scene) const;

private:
    CameraManager();
    ~CameraManager();
    CameraManager(const CameraManager&)            = delete;
    CameraManager* operator=(const CameraManager&) = delete;

private:
    std::unordered_map<std::string, IConstantBuffer<CameraTransform>> cameraBuffers_;
};

} // namespace OriGine
