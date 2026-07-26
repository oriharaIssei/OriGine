#pragma once

/// parent
#include "MeshRenderer.h"

/// math
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include <DirectXMath.h>

namespace OriGine {

/// <summary>
/// 天空箱用頂点データ
/// </summary>
struct SkyboxVertex {
    Vec4f position;
};
/// <summary>
/// 天空箱用マテリアルデータ
/// </summary>
struct SkyboxMaterial {
    friend void to_json(nlohmann::json& _j, const SkyboxMaterial& _comp);
    friend void from_json(const nlohmann::json& _j, SkyboxMaterial& _comp);
    Vec4f color;

    struct ConstantBuffer {
        Vec4f color;
        ConstantBuffer& operator=(const SkyboxMaterial& _material) {
            this->color = _material.color;
            return *this;
        }
    };
};

/// <summary>
/// 天空箱レンダラーコンポーネント。
/// 立方体1つ分の頂点/インデックスを持つ固定形状のメッシュ(meshGroup_)と、
/// 座標変換用(transformBuff_)・色用(materialBuff_)の2つの定数バッファを持つ。
/// いずれもCPU側のopenData_を書き換えただけでは反映されず、ConvertToBuffer()/TransferData()を
/// 呼んだタイミングでGPU側のMap済みメモリへコピーされる
/// </summary>
class SkyboxRenderer
    : public MeshRenderer<Mesh<SkyboxVertex>, SkyboxVertex> {
    friend void to_json(nlohmann::json& _j, const SkyboxRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, SkyboxRenderer& _comp);

public:
    SkyboxRenderer() : MeshRenderer() {}
    ~SkyboxRenderer() {}

    /// <summary>
    /// メッシュ・バッファの生成やテクスチャ読み込みなど、天空箱描画に必要な初期化を行う
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _hostEntity) override;
    /// <summary>
    /// エディタ上で天空箱のテクスチャ・マテリアルを編集するGUIを描画する
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;

private:
    std::string filePath_; // 読み込んだキューブマップテクスチャのファイルパス
    IConstantBuffer<Transform> transformBuff_; // 座標変換用定数バッファ
    IConstantBuffer<SkyboxMaterial> materialBuff_; // 天空箱マテリアル用定数バッファ

    size_t textureIndex_ = 0; // 読み込んだテクスチャのインデックス

public:
    void SetTextureIndex(size_t _textureIndex) {
        textureIndex_ = _textureIndex;
    }
    size_t GetTextureIndex() const {
        return textureIndex_;
    }
    void SetFilePath(const std::string& _filePath) {
        filePath_ = _filePath;
    }
    const std::string& GetFilePath() const {
        return filePath_;
    }

    IConstantBuffer<SkyboxMaterial>& GetMaterialBuff() {
        return materialBuff_;
    }
    IConstantBuffer<Transform>& GetTransformBuff() {
        return transformBuff_;
    }
};

void to_json(nlohmann::json& _j, const SkyboxMaterial& _comp);
void from_json(const nlohmann::json& _j, SkyboxMaterial& _comp);

} // namespace OriGine
