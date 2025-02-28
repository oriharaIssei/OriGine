#include "Sprite.h"

/// algorithm
#include <algorithm>

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxFunctionHelper.h"
#include <directX12/ShaderCompiler.h>
// assets
#include "texture/TextureManager.h"
// lib
#include "logger/Logger.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void SpriteRenderer::Init() {
    // buffer作成
    spriteBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    // メッシュの初期化
    meshGroup_ = std::make_shared<std::vector<SpriteMesh>>();
    meshGroup_->push_back(SpriteMesh());

    SpriteMesh& mesh = meshGroup_->at(0);
    mesh.Init(4, 6);
    // indexData
    mesh.indexData[0] = 0;
    mesh.indexData[1] = 1;
    mesh.indexData[2] = 2;
    mesh.indexData[3] = 1;
    mesh.indexData[4] = 3;
    mesh.indexData[5] = 2;

    // テクスチャの読み込みとサイズの適応
    if (!texturePath_.empty()) {
        textureNumber_ = TextureManager::LoadTexture(texturePath_, [this](uint32_t index) {
            const DirectX::TexMetadata& texData = TextureManager::getTexMetadata(index);
            if (textureSize_.lengthSq() == 0.0f) {
                textureSize_ = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
            }
            if (spriteBuff_->scale_.lengthSq() == 0.0f) {
                spriteBuff_->scale_ = textureSize_;
            }
        });
    }
}

void SpriteRenderer::Finalize() {
}

void SpriteRenderer::Update(const Matrix4x4& _viewPortMat) {
    //-------------------------------- ConstBufferの更新 --------------------------------//
    {
        spriteBuff_->Update(_viewPortMat);

        spriteBuff_.ConvertToBuffer();
    }
    //-------------------------------- メッシュの更新 --------------------------------//
    float left   = -anchorPoint_[X];
    float right  = 1.0f - anchorPoint_[X];
    float top    = -anchorPoint_[Y];
    float bottom = 1.0f - anchorPoint_[Y];

    if (isFlipX_) {
        left  = -left;
        right = -right;
    }
    if (isFlipY_) {
        top    = -top;
        bottom = -bottom;
    }

    SpriteMesh& mesh     = meshGroup_->at(0);
    mesh.vertData[0].pos = {left, bottom, 0.0f, 1.0f};
    mesh.vertData[1].pos = {left, top, 0.0f, 1.0f};
    mesh.vertData[2].pos = {right, bottom, 0.0f, 1.0f};
    mesh.vertData[3].pos = {right, top, 0.0f, 1.0f};

    float texLeft   = textureLeftTop_[X] / textureSize_[X];
    float texRight  = (textureLeftTop_[X] + textureSize_[X]) / textureSize_[X];
    float texTop    = textureLeftTop_[Y] / textureSize_[Y];
    float texBottom = (textureLeftTop_[Y] + textureSize_[Y]) / textureSize_[Y];

    mesh.vertData[0].texcoord = {texLeft, texBottom};
    mesh.vertData[1].texcoord = {texLeft, texTop};
    mesh.vertData[2].texcoord = {texRight, texBottom};
    mesh.vertData[3].texcoord = {texRight, texTop};
}
