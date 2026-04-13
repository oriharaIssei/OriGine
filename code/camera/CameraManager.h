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
/// シーンごとに CameraTransform の定数バッファを保持・管理する.
/// </summary>
class CameraManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する.
    /// </summary>
    /// <returns>インスタンスのポインタ</returns>
    static CameraManager* GetInstance();

    /// <summary>
    /// 初期化を行う.
    /// 保持しているバッファ情報をクリアする.
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理を行う.
    /// 登録されているすべての定数バッファを破棄する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 指定したシーン名でカメラを登録する.
    /// </summary>
    /// <param name="_sceneName">登録するシーン名</param>
    /// <returns>登録に成功したか</returns>
    bool RegisterSceneCamera(const std::string& _sceneName);

    /// <summary>
    /// 指定したシーン名のカメラ登録を解除する.
    /// </summary>
    /// <param name="_sceneName">解除するシーン名</param>
    /// <returns>解除に成功したか</returns>
    bool UnregisterSceneCamera(const std::string& _sceneName);

    /// <summary>
    /// 指定したシーンのカメラを登録する.
    /// </summary>
    /// <param name="_scene">登録するシーンのポインタ</param>
    /// <returns>登録に成功したか</returns>
    bool RegisterSceneCamera(Scene* _scene);

    /// <summary>
    /// 指定したシーンのカメラ登録を解除する.
    /// </summary>
    /// <param name="_scene">解除するシーンのポインタ</param>
    /// <returns>解除に成功したか</returns>
    bool UnregisterSceneCamera(Scene* _scene);

    /// <summary>
    /// 指定したシーンのカメラトランスフォームを設定する.
    /// </summary>
    /// <param name="_sceneName">設定対象のシーン名</param>
    /// <param name="_transform">設定するトランスフォーム情報</param>
    /// <returns>設定に成功したか</returns>
    bool SetTransform(const std::string& _sceneName, const CameraTransform& _transform);

    /// <summary>
    /// 指定したシーンのカメラトランスフォームを設定する.
    /// </summary>
    /// <param name="_scene">設定対象のシーン</param>
    /// <param name="_transform">設定するトランスフォーム情報</param>
    /// <returns>設定に成功したか</returns>
    bool SetTransform(Scene* _scene, const CameraTransform& _transform);

    /// <summary>
    /// カメラデータを定数バッファに転送する.
    /// </summary>
    /// <param name="_sceneName">対象のシーン名</param>
    /// <returns>転送に成功したか</returns>
    bool DataConvertToBuffer(const std::string& _sceneName);

    /// <summary>
    /// カメラデータを定数バッファに転送する.
    /// </summary>
    /// <param name="_scene">対象のシーン</param>
    /// <returns>転送に成功したか</returns>
    bool DataConvertToBuffer(Scene* _scene);

    /// <summary>
    /// 定数バッファをルートパラメータにセットする.
    /// </summary>
    /// <param name="_sceneName">対象のシーン名</param>
    /// <param name="_cmdList">コマンドリスト</param>
    /// <param name="_rootParameterNum">ルートパラメータ番号</param>
    /// <returns>セットに成功したか</returns>
    bool SetBufferForRootParameter(
        const std::string& _sceneName,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList,
        uint32_t _rootParameterNum);

    /// <summary>
    /// 定数バッファをルートパラメータにセットする.
    /// </summary>
    /// <param name="_scene">対象のシーン</param>
    /// <param name="_cmdList">コマンドリスト</param>
    /// <param name="_rootParameterNum">ルートパラメータ番号</param>
    /// <returns>セットに成功したか</returns>
    bool SetBufferForRootParameter(
        Scene* _scene,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList,
        uint32_t _rootParameterNum);

    /// <summary>
    /// 指定したシーンの現在のカメラトランスフォームを取得する.
    /// </summary>
    /// <param name="_sceneName">対象のシーン名</param>
    /// <returns>カメラトランスフォーム</returns>
    CameraTransform GetTransform(const std::string& _sceneName) const;

    /// <summary>
    /// 指定したシーンの現在のカメラトランスフォームを取得する.
    /// </summary>
    /// <param name="_scene">対象のシーン</param>
    /// <returns>カメラトランスフォーム</returns>
    CameraTransform GetTransform(Scene* _scene) const;

private:
    CameraManager();
    ~CameraManager();
    CameraManager(const CameraManager&)            = delete;
    CameraManager* operator=(const CameraManager&) = delete;

private:
    /// <summary>シーン名と定数バッファの対応マップ</summary>
    std::unordered_map<std::string, IConstantBuffer<CameraTransform>> cameraBuffers_;
};

} // namespace OriGine
