#include "Sprite.h"

/// algorithm
#include <algorithm>

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#include "Engine.h"
// directX12Object
#include "directX12/DxFunctionHelper.h"
#include <directX12/ShaderCompiler.h>
// asSets
#include "texture/TextureManager.h"

#include "logger/Logger.h"
#include "myFileSystem/MyFileSystem.h"

/// math
#include "math/Matrix4x4.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SpriteRenderer::Initialize(Entity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    // buffer作成
    spriteBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

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
            const DirectX::TexMetadata& texData = TextureManager::GetTexMetadata(index);
            if (textureSize_.lengthSq() == 0.0f) {
                textureSize_ = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
            }
            if (size_.lengthSq() == 0.0f) {
                size_ = textureSize_;
            }
        });
    }

    CalculateWindowRatioPosAndSize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
}

void SpriteRenderer::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    auto realNumberAfterFunc = [this](Vector<2, float>* /*_newVal*/) {
        CalculatePosRatioAndSizeRatio();
    };
    auto ratioAfterFunc = [this](Vector<2, float>* /*_newVal*/) {
        CalculateWindowRatioPosAndSize(defaultWindowSize_);
    };

    ImGui::Text("Texture Path : %s", texturePath_.c_str());
    ImGui::SameLine();

    auto askLoad = [this]([[maybe_unused]] const std::string& _parentLabel) {
        bool askLoad      = false;
        std::string label = "LoadTexture##" + _parentLabel;
        askLoad |= ImGui::Button(label.c_str());
        static ImVec2 textureButtonSize = {32.f, 32.f};
        askLoad |= ImGui::ImageButton(
            reinterpret_cast<ImTextureID>(TextureManager::GetDescriptorGpuHandle(textureNumber_).ptr),
            textureButtonSize,
            {0, 0}, {1, 1},
            8);

        return askLoad;
    };

    if (askLoad(_parentLabel)) {
        std::string directory;
        std::string fileName;
        if (myFs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            // コマンドを作成
            auto command = std::make_unique<SetterCommand<std::string>>(
                &texturePath_,
                kApplicationResourceDirectory + "/" + directory + "/" + fileName,
                [this](std::string* _fileName) {
                    // テクスチャの読み込み
                    textureNumber_ = TextureManager::LoadTexture(*_fileName, [this](uint32_t loadIndex) {
                        const DirectX::TexMetadata& texData = TextureManager::GetTexMetadata(loadIndex);
                        textureSize_                        = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
                        size_                               = textureSize_;
                    });
                });

            OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }

    ImGui::Spacing();

    CheckBoxCommand("isRendering##" + _parentLabel, isRender_);
    ImGui::Text("RenderingPriority");
    DragGuiCommand("##RenderingPriority" + _parentLabel, renderPriority_, 1, 0, 1000, "%d");

    ImGui::Text("AnchorPoint");
    DragGuiVectorCommand("##AnchorPoint" + _parentLabel, anchorPoint_, 0.01f, -1.0f, 1.0f);

    ImGui::Text("TextureLeftTop");
    DragGuiVectorCommand("##TextureLeftTop" + _parentLabel, textureLeftTop_, 1.0f, 0.0f, 1000.0f);

    ImGui::Text("TextureSize");
    DragGuiVectorCommand("##TextureSize" + _parentLabel, textureSize_, 1.0f, 0.0f, 1000.0f);

    ImGui::Text("Size");
    if (DragGuiVectorCommand<2, float>("##Size" + _parentLabel, size_, 1.f, 0.0f, 0.f, "%.1f", realNumberAfterFunc)) {
        CalculatePosRatioAndSizeRatio();
    }

    ImGui::Text("WindowRatioSize");
    ImGui::SameLine();
    if (DragGuiVectorCommand<2, float>("##WindowRatioSize" + _parentLabel, windowRatioSize_, 0.001f, 0.0f, 0.f, "%.4f", ratioAfterFunc)) {
        CalculateWindowRatioPosAndSize();
    }

    ImGui::Spacing();

    ImGui::Text("Flip");
    CheckBoxCommand("X##flip" + _parentLabel, isFlipX_);
    CheckBoxCommand("Y##flip" + _parentLabel, isFlipY_);

    ImGui::Spacing();

    ImGui::Text("Color");
    ColorEditGuiCommand("##Color" + _parentLabel, spriteBuff_->color_);

    ImGui::Spacing();

    if (ImGui::TreeNode("SpriteTransform")) {

        ImGui::Text("Scale");
        DragGuiVectorCommand("##Scale" + _parentLabel, spriteBuff_->scale_, 0.01f);
        ImGui::Text("Rotate");
        DragGuiCommand("##Rotate" + _parentLabel, spriteBuff_->rotate_, 0.01f);
        ImGui::Text("Translate");
        if (DragGuiVectorCommand<2, float>("##Translate" + _parentLabel, spriteBuff_->translate_, 0.01f, 0.0f, 0.f, "%.3f", realNumberAfterFunc)) {
            CalculatePosRatioAndSizeRatio();
        }
        ImGui::Text("WindowRatioPos");
        ImGui::SameLine();
        if (DragGuiVectorCommand<2, float>("##WindowRatioPos" + _parentLabel, windowRatioPos_, 0.001f, 0.0f, 0.f, "%.4f", ratioAfterFunc)) {
            CalculateWindowRatioPosAndSize();
        }

        ImGui::TreePop();
    }

    ImGui::Spacing();

    if (ImGui::TreeNode("UVTransform")) {
        ImGui::Text("UVScale");
        DragGuiVectorCommand("##UVScale" + _parentLabel, spriteBuff_->uvScale_, 0.01f);
        ImGui::Text("UVRotate");
        DragGuiCommand("##UVRotate" + _parentLabel, spriteBuff_->uvRotate_, 0.01f);
        ImGui::Text("UVTranslate");
        DragGuiVectorCommand("##UVTranslate" + _parentLabel, spriteBuff_->uvTranslate_, 0.01f);

        ImGui::TreePop();
    }

#endif // _DEBUG
}

void SpriteRenderer::Finalize() {
    MeshRenderer::Finalize();
    spriteBuff_.Finalize();
}

void SpriteRenderer::CalculateWindowRatioPosAndSize() {
    if (windowRatioSize_.lengthSq() != 0.0f) {
        size_ = {defaultWindowSize_[X] * windowRatioSize_[X], defaultWindowSize_[Y] * windowRatioSize_[Y]};
    }
    if (windowRatioPos_.lengthSq() != 0.0f) {
        spriteBuff_->translate_ = {defaultWindowSize_[X] * windowRatioPos_[X], defaultWindowSize_[Y] * windowRatioPos_[Y]};
    }
}

void SpriteRenderer::CalculateWindowRatioPosAndSize(const Vec2f& _newWindowSize) {
    defaultWindowSize_ = _newWindowSize;
    CalculateWindowRatioPosAndSize();
}

void SpriteRenderer::CalculatePosRatioAndSizeRatio() {
    if (defaultWindowSize_.lengthSq() == 0.0f) {
        return;
    }
    if (size_.lengthSq() != 0.0f) {
        windowRatioSize_ = {size_[X] / defaultWindowSize_[X], size_[Y] / defaultWindowSize_[Y]};
    }
    if (spriteBuff_->translate_.lengthSq() != 0.0f) {
        windowRatioPos_ = {spriteBuff_->translate_[X] / defaultWindowSize_[X], spriteBuff_->translate_[Y] / defaultWindowSize_[Y]};
    }
}

void SpriteRenderer::CalculatePosRatioAndSizeRatio(const Vec2f& _newWindowSize) {
    defaultWindowSize_ = _newWindowSize;
    CalculatePosRatioAndSizeRatio();
}

void SpriteRenderer::SetTexture(const std::string& _texturePath, bool _applyTextureSize) {
    texturePath_ = _texturePath;
    // テクスチャの読み込みとサイズの適応
    if (_applyTextureSize) {
        textureNumber_ = TextureManager::LoadTexture(texturePath_, [this](uint32_t loadIndex) {
            const DirectX::TexMetadata& texData = TextureManager::GetTexMetadata(loadIndex);
            textureSize_                        = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
            size_                               = textureSize_;
        });
    } else {
        textureNumber_ = TextureManager::LoadTexture(texturePath_);
    }
}

void SpriteRenderer::UpdateBuffer(const Matrix4x4& _viewPortMat) {
    //-------------------------------- ConstBufferの更新 --------------------------------//
    {
        spriteBuff_->Update(_viewPortMat);

        spriteBuff_.ConvertToBuffer();
    }
    //-------------------------------- メッシュの更新 --------------------------------//
    float left   = -anchorPoint_[X] * size_[X];
    float right  = (1.0f - anchorPoint_[X]) * size_[X];
    float top    = -anchorPoint_[Y] * size_[Y];
    float bottom = (1.0f - anchorPoint_[Y]) * size_[Y];

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

void OriGine::to_json(nlohmann::json& j, const SpriteRenderer& r) {
    j = nlohmann::json{
        {"isRender", r.isRender_},
        {"renderingPriority", r.renderPriority_},
        {"texturePath", r.texturePath_},
        {"textureLeftTop", r.textureLeftTop_},
        {"textureSize", r.textureSize_},
        {"anchorPoint", r.anchorPoint_},
        {"isFlipX", r.isFlipX_},
        {"isFlipY", r.isFlipY_},
        {"color", r.spriteBuff_->color_},
        {"scale", r.spriteBuff_->scale_},
        {"size", r.size_},
        {"defaultWindowSize", r.defaultWindowSize_},
        {"windowRatioPos", r.windowRatioPos_},
        {"windowRatioSize", r.windowRatioSize_},
        {"rotate", r.spriteBuff_->rotate_},
        {"translate", r.spriteBuff_->translate_},
        {"uvScale", r.spriteBuff_->uvScale_},
        {"uvRotate", r.spriteBuff_->uvRotate_},
        {"uvTranslate", r.spriteBuff_->uvTranslate_}};
}

void OriGine::from_json(const nlohmann::json& j, SpriteRenderer& r) {
    j.at("isRender").get_to(r.isRender_);
    j.at("renderingPriority").get_to(r.renderPriority_);

    j.at("texturePath").get_to(r.texturePath_);

    j.at("textureLeftTop").get_to(r.textureLeftTop_);
    j.at("textureSize").get_to(r.textureSize_);
    j.at("size").get_to(r.size_);
    j.at("anchorPoint").get_to(r.anchorPoint_);

    j.at("isFlipX").get_to(r.isFlipX_);
    j.at("isFlipY").get_to(r.isFlipY_);

    j.at("color").get_to(r.spriteBuff_->color_);

    j.at("scale").get_to(r.spriteBuff_->scale_);
    j.at("rotate").get_to(r.spriteBuff_->rotate_);
    j.at("translate").get_to(r.spriteBuff_->translate_);

    j.at("uvScale").get_to(r.spriteBuff_->uvScale_);
    j.at("uvRotate").get_to(r.spriteBuff_->uvRotate_);
    j.at("uvTranslate").get_to(r.spriteBuff_->uvTranslate_);

    if (j.find("defaultWindowSize") != j.end()) {
        j.at("defaultWindowSize").get_to(r.defaultWindowSize_);
    } else {
        r.defaultWindowSize_ = Engine::GetInstance()->GetWinApp()->GetWindowSize();
    }
    if (j.find("windowRatioSize") != j.end()) {
        j.at("windowRatioSize").get_to(r.windowRatioSize_);
        if (r.defaultWindowSize_.lengthSq() != 0.0f) {
            r.size_ = {r.defaultWindowSize_[X] * r.windowRatioSize_[X], r.defaultWindowSize_[Y] * r.windowRatioSize_[Y]};
        }
    } else {
        r.windowRatioSize_ = {r.size_[X] / r.defaultWindowSize_[X], r.size_[Y] / r.defaultWindowSize_[Y]};
    }
    if (j.find("windowRatioPos") != j.end()) {
        j.at("windowRatioPos").get_to(r.windowRatioPos_);
        if (r.defaultWindowSize_.lengthSq() != 0.0f) {
            r.spriteBuff_->translate_ = {r.defaultWindowSize_[X] * r.windowRatioPos_[X], r.defaultWindowSize_[Y] * r.windowRatioPos_[Y]};
        }
    } else {
        r.windowRatioPos_ = {r.spriteBuff_->translate_[X] / r.defaultWindowSize_[X], r.spriteBuff_->translate_[Y] / r.defaultWindowSize_[Y]};
    }
}
