#pragma once

/// math
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

namespace OriGine {

/// <summary>
/// 1頂点を表すデータ(テクスチャあり)
/// </summary>
struct TextureVertexData {
    TextureVertexData* operator=(const TextureVertexData& vert) {
        this->pos      = vert.pos;
        this->texCoord = vert.texCoord;
        this->normal   = vert.normal;
        return this;
    }

    Vec4f pos;
    Vec2f texCoord;
    Vec3f normal;

    bool operator==(const TextureVertexData& vert) {
        if (this->pos != vert.pos) {
            return false;
        }
        if (this->texCoord != vert.texCoord) {
            return false;
        }
        if (this->normal != vert.normal) {
            return false;
        }
        return true;
    }
};
/// <summary>
/// 1頂点を表すデータ(テクスチャなし)
/// </summary>
struct PrimitiveVertexData {
    PrimitiveVertexData(const TextureVertexData& vert) {
        this->pos    = vert.pos;
        this->normal = vert.normal;
    }

    Vec4f pos;
    Vec3f normal;

    PrimitiveVertexData* operator=(const PrimitiveVertexData& vert) {
        this->pos    = vert.pos;
        this->normal = vert.normal;
        return this;
    }
    PrimitiveVertexData* operator=(const TextureVertexData& vert) {
        this->pos    = vert.pos;
        this->normal = vert.normal;
        return this;
    }
};

/// <summary>
/// 色付き頂点データ
/// </summary>
struct ColorVertexData {
    Vec4f pos   = {0.f, 0.f, 0.f, 1.f};
    Vec4f color = kWhite;

    ColorVertexData* operator=(const ColorVertexData& vert) {
        this->pos   = vert.pos;
        this->color = vert.color;
        return this;
    }
};

/// <summary>
/// 1頂点を表すデータ(テクスチャ と 色あり)
/// </summary>
struct TextureColorVertexData {
    Vec4f pos;
    Vec2f texCoord;
    Vec3f normal;
    Vec4f color = kWhite;
    TextureColorVertexData* operator=(const TextureColorVertexData& vert) {
        this->pos      = vert.pos;
        this->texCoord = vert.texCoord;
        this->normal   = vert.normal;
        this->color    = vert.color;
        return this;
    }

    bool operator==(const TextureColorVertexData& vert) {
        if (this->pos != vert.pos) {
            return false;
        }
        if (this->texCoord != vert.texCoord) {
            return false;
        }
        if (this->normal != vert.normal) {
            return false;
        }
        if (this->color != vert.color) {
            return false;
        }

        return true;
    }
};

} // namespace OriGine
