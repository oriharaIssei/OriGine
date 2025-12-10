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
    friend void to_json(nlohmann::json& j, const SkyboxMaterial& c);
    friend void from_json(const nlohmann::json& j, SkyboxMaterial& c);
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
    friend void to_json(nlohmann::json& j, const SkyboxRenderer& c);
    friend void from_json(const nlohmann::json& j, SkyboxRenderer& c);

public:
    SkyboxRenderer() : MeshRenderer() {}
    ~SkyboxRenderer() {}

    void Initialize(Entity* _hostEntity) override;
    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

private:
    std::string filePath_;
    IConstantBuffer<Transform> transformBuff_;
    IConstantBuffer<SkyboxMaterial> materialBuff_;

    int32_t textureIndex_ = 0;

public:
    void SetTextureIndex(int32_t _textureIndex) {
        textureIndex_ = _textureIndex;
    }
    int32_t GetTextureIndex() const {
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

void to_json(nlohmann::json& j, const SkyboxMaterial& c);
void from_json(const nlohmann::json& j, SkyboxMaterial& c);

} // namespace OriGine
