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
/// 天空箱レンダラーコンポーネント
/// </summary>
class SkyboxRenderer
    : public MeshRenderer<Mesh<SkyboxVertex>, SkyboxVertex> {
    friend void to_json(nlohmann::json& _j, const SkyboxRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, SkyboxRenderer& _comp);

public:
    SkyboxRenderer() : MeshRenderer() {}
    ~SkyboxRenderer() {}

    void Initialize(Scene* _scene, EntityHandle _hostEntity) override;
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

private:
    std::string filePath_;
    IConstantBuffer<Transform> transformBuff_;
    IConstantBuffer<SkyboxMaterial> materialBuff_;

    size_t textureIndex_ = 0;

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
