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
#include "myFileSystem/MyFileSystem.h"

/// math
#include "math/Matrix4x4.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void SpriteRenderer::Initialize(GameEntity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    // buffer作成
    spriteBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    // メッシュの初期化
    meshGroup_ = std::make_shared<std::vector<SpriteMesh>>();
    meshGroup_->push_back(SpriteMesh());

    SpriteMesh& mesh = meshGroup_->at(0);
    mesh.Initialize(4, 6);
    // indexData
    mesh.indexes_[0] = 0;
    mesh.indexes_[1] = 1;
    mesh.indexes_[2] = 2;
    mesh.indexes_[3] = 1;
    mesh.indexes_[4] = 3;
    mesh.indexes_[5] = 2;
    mesh.TransferData();

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

bool SpriteRenderer::Edit() {
#ifdef _DEBUG
    bool isChange = false;

    ImGui::Text("Texture Path : %s", texturePath_.c_str());
    ImGui::SameLine();
    if (ImGui::Button("...")) {
        std::string directory;
        std::string fileName;
        if (myFs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            texturePath_ = kApplicationResourceDirectory + "/" + directory + "/" + fileName;

            // テクスチャの読み込みとサイズの適応
            textureNumber_ = TextureManager::LoadTexture(texturePath_, [this](uint32_t loadIndex) {
                const DirectX::TexMetadata& texData = TextureManager::getTexMetadata(loadIndex);
                textureSize_                        = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
                spriteBuff_->scale_                 = textureSize_;
            });

            isChange = true;
        }
    }

    ImGui::Spacing();

    ImGui::Text("RenderingPriority");
    isChange |= ImGui::DragInt("##RenderingPriority", &renderPriority_, 1.0f, 0, 1000);

    ImGui::Text("TextureSize");
    isChange |= ImGui::DragFloat2("##TextureSize", textureSize_.v, 1.0f, 0.0f, 1000.0f);

    ImGui::Text("TextureLeftTop");
    isChange |= ImGui::DragFloat2("##TextureLeftTop", textureLeftTop_.v, 1.0f, 0.0f, 1000.0f);

    ImGui::Text("AnchorPoint");
    isChange |= ImGui::DragFloat2("##AnchorPoint", anchorPoint_.v, 0.01f, -1.0f, 1.0f);

    ImGui::Spacing();

    ImGui::Text("Flip");
    isChange |= ImGui::Checkbox("FlipX", &isFlipX_);
    ImGui::SameLine();
    isChange |= ImGui::Checkbox("FlipY", &isFlipY_);

    ImGui::Spacing();

    ImGui::Text("Color");
    isChange |= ImGui::ColorEdit4("##Color", spriteBuff_->color_.v);

    if (ImGui::TreeNode("UVTransform")) {
        ImGui::Text("UVScale");
        isChange |= ImGui::DragFloat2("##UVScale", spriteBuff_->uvScale_.v, 0.01f, 0.0f, 1000.0f);
        ImGui::Text("UVRotate");
        isChange |= ImGui::DragFloat("##UVRotate", &spriteBuff_->uvRotate_, 0.01f, 0.0f, 1000.0f);
        ImGui::Text("UVTranslate");
        isChange |= ImGui::DragFloat2("##UVTranslate", spriteBuff_->uvTranslate_.v, 0.01f, 0.0f, 1000.0f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("SpriteTransform")) {
        ImGui::Text("Size(Scale)");
        isChange |= ImGui::DragFloat2("##Scale", spriteBuff_->scale_.v, 0.01f, 0.0f, 1000.0f);
        ImGui::Text("Rotate");
        isChange |= ImGui::DragFloat("##Rotate", &spriteBuff_->rotate_, 0.01f, 0.0f, 1000.0f);
        ImGui::Text("Translate");
        isChange |= ImGui::DragFloat2("##Translate", spriteBuff_->translate_.v, 0.01f, 0.0f, 1000.0f);
        ImGui::TreePop();
    }

    return isChange;

#else
    return false;
#endif // _DEBUG
}

void SpriteRenderer::Finalize() {
    MeshRenderer::Finalize();
    spriteBuff_.Finalize();
}

void SpriteRenderer::setTexture(const std::string& _texturePath, bool _applyTextureSize) {
    texturePath_ = _texturePath;
    // テクスチャの読み込みとサイズの適応
    if (_applyTextureSize) {
        textureNumber_ = TextureManager::LoadTexture(texturePath_, [this](uint32_t loadIndex) {
            const DirectX::TexMetadata& texData = TextureManager::getTexMetadata(loadIndex);
            textureSize_                        = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
            spriteBuff_->scale_                 = textureSize_;
        });
    } else {
        textureNumber_ = TextureManager::LoadTexture(texturePath_);
    }
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

    SpriteMesh& mesh      = meshGroup_->at(0);
    mesh.vertexes_[0].pos = {left, bottom, 0.0f, 1.0f};
    mesh.vertexes_[1].pos = {left, top, 0.0f, 1.0f};
    mesh.vertexes_[2].pos = {right, bottom, 0.0f, 1.0f};
    mesh.vertexes_[3].pos = {right, top, 0.0f, 1.0f};

    float texLeft   = textureLeftTop_[X] / textureSize_[X];
    float texRight  = (textureLeftTop_[X] + textureSize_[X]) / textureSize_[X];
    float texTop    = textureLeftTop_[Y] / textureSize_[Y];
    float texBottom = (textureLeftTop_[Y] + textureSize_[Y]) / textureSize_[Y];

    mesh.vertexes_[0].texcoord = {texLeft, texBottom};
    mesh.vertexes_[1].texcoord = {texLeft, texTop};
    mesh.vertexes_[2].texcoord = {texRight, texBottom};
    mesh.vertexes_[3].texcoord = {texRight, texTop};

    mesh.TransferData();
}

void to_json(nlohmann::json& j, const SpriteRenderer& r) {
    j = nlohmann::json{
        {"renderingPriority", r.renderPriority_},
        {"texturePath", r.texturePath_},
        {"textureLeftTop", r.textureLeftTop_},
        {"textureSize", r.textureSize_},
        {"anchorPoint", r.anchorPoint_},
        {"isFlipX", r.isFlipX_},
        {"isFlipY", r.isFlipY_},
        {"scale", r.spriteBuff_->scale_},
        {"rotate", r.spriteBuff_->rotate_},
        {"translate", r.spriteBuff_->translate_},
        {"uvScale", r.spriteBuff_->uvScale_},
        {"uvRotate", r.spriteBuff_->uvRotate_},
        {"uvTranslate", r.spriteBuff_->uvTranslate_}};
}

void from_json(const nlohmann::json& j, SpriteRenderer& r) {
    j.at("renderingPriority").get_to(r.renderPriority_);
    j.at("texturePath").get_to(r.texturePath_);
    j.at("textureLeftTop").get_to(r.textureLeftTop_);
    j.at("textureSize").get_to(r.textureSize_);
    j.at("anchorPoint").get_to(r.anchorPoint_);
    j.at("isFlipX").get_to(r.isFlipX_);
    j.at("isFlipY").get_to(r.isFlipY_);
    j.at("scale").get_to(r.spriteBuff_->scale_);
    j.at("rotate").get_to(r.spriteBuff_->rotate_);
    j.at("translate").get_to(r.spriteBuff_->translate_);
    j.at("uvScale").get_to(r.spriteBuff_->uvScale_);
    j.at("uvRotate").get_to(r.spriteBuff_->uvRotate_);
    j.at("uvTranslate").get_to(r.spriteBuff_->uvTranslate_);
}
