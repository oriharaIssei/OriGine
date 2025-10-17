#pragma once

/// math
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

/// <summary>
/// 1頂点を表すデータ(テクスチャあり)
/// </summary>
struct TextureVertexData {
    Vec4f pos;
    Vec2f texCoord;
    Vec3f normal;
    TextureVertexData* operator=(const TextureVertexData& vert) {
        this->pos      = vert.pos;
        this->texCoord = vert.texCoord;
        this->normal   = vert.normal;
        return this;
    }

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
    Vec4f pos;
    Vec3f normal;
    PrimitiveVertexData(const TextureVertexData& vert) {
        this->pos    = vert.pos;
        this->normal = vert.normal;
    }
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

struct ColorVertexData {
    Vec4f pos = {0.f, 0.f, 0.f, 1.f};
    Vec4f color;

    ColorVertexData* operator=(const ColorVertexData& vert) {
        this->pos   = vert.pos;
        this->color = vert.color;
        return this;
    }
};
