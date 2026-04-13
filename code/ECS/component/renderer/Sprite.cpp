#include "Sprite.h"

/// algorithm
#include <algorithm>

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#include "Engine.h"
// asset
#include "asset/TextureAsset.h"
// directX12Object
#include "directX12/DxFunctionHelper.h"
#include <directX12/ShaderCompiler.h>
// assets
#include "asset/AssetSystem.h"

#include "logger/Logger.h"
#include "myFileSystem/MyFileSystem.h"

/// math
#include "math/Matrix4x4.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SpriteRenderer::Initialize(Scene* _scene, EntityHandle _hostEntity) {
    MeshRenderer::Initialize(_scene, _hostEntity);

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
        textureIndex_ = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(texturePath_);
        const DirectX::TexMetadata& texData = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(textureIndex_).metaData;
        if (textureSize_.lengthSq() == 0.0f) {
            textureSize_ = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
        }
        if (size_.lengthSq() == 0.0f) {
            size_ = textureSize_;
        }
    }

    CalculateWindowRatioPosAndSize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
}

void SpriteRenderer::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
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
            reinterpret_cast<ImTextureID>(AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(textureIndex_).srv.GetGpuHandle().ptr),
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
                    textureIndex_ = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(*_fileName);
                    const DirectX::TexMetadata& texData = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(textureIndex_).metaData;
                    textureSize_                        = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
                    size_                               = textureSize_;
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
        textureIndex_                      = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(texturePath_);
        const DirectX::TexMetadata& texData = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(textureIndex_).metaData;
        if (textureSize_.lengthSq() == 0.0f) {
            textureSize_ = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
        }
        if (size_.lengthSq() == 0.0f) {
            size_ = textureSize_;
        }
    } else {
        textureIndex_ = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(texturePath_);
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

void OriGine::to_json(nlohmann::json& _j, const SpriteRenderer& _comp) {
    _j = nlohmann::json{
        {"isRender", _comp.isRender_},
        {"renderingPriority", _comp.renderPriority_},
        {"texturePath", _comp.texturePath_},
        {"textureLeftTop", _comp.textureLeftTop_},
        {"textureSize", _comp.textureSize_},
        {"anchorPoint", _comp.anchorPoint_},
        {"isFlipX", _comp.isFlipX_},
        {"isFlipY", _comp.isFlipY_},
        {"color", _comp.spriteBuff_->color_},
        {"scale", _comp.spriteBuff_->scale_},
        {"size", _comp.size_},
        {"defaultWindowSize", _comp.defaultWindowSize_},
        {"windowRatioPos", _comp.windowRatioPos_},
        {"windowRatioSize", _comp.windowRatioSize_},
        {"rotate", _comp.spriteBuff_->rotate_},
        {"translate", _comp.spriteBuff_->translate_},
        {"uvScale", _comp.spriteBuff_->uvScale_},
        {"uvRotate", _comp.spriteBuff_->uvRotate_},
        {"uvTranslate", _comp.spriteBuff_->uvTranslate_}};
}

void OriGine::from_json(const nlohmann::json& _j, SpriteRenderer& _comp) {
    _j.at("isRender").get_to(_comp.isRender_);
    _j.at("renderingPriority").get_to(_comp.renderPriority_);

    _j.at("texturePath").get_to(_comp.texturePath_);

    _j.at("textureLeftTop").get_to(_comp.textureLeftTop_);
    _j.at("textureSize").get_to(_comp.textureSize_);
    _j.at("size").get_to(_comp.size_);
    _j.at("anchorPoint").get_to(_comp.anchorPoint_);

    _j.at("isFlipX").get_to(_comp.isFlipX_);
    _j.at("isFlipY").get_to(_comp.isFlipY_);

    _j.at("color").get_to(_comp.spriteBuff_->color_);

    _j.at("scale").get_to(_comp.spriteBuff_->scale_);
    _j.at("rotate").get_to(_comp.spriteBuff_->rotate_);
    _j.at("translate").get_to(_comp.spriteBuff_->translate_);

    _j.at("uvScale").get_to(_comp.spriteBuff_->uvScale_);
    _j.at("uvRotate").get_to(_comp.spriteBuff_->uvRotate_);
    _j.at("uvTranslate").get_to(_comp.spriteBuff_->uvTranslate_);

    if (_j.find("defaultWindowSize") != _j.end()) {
        _j.at("defaultWindowSize").get_to(_comp.defaultWindowSize_);
    } else {
        _comp.defaultWindowSize_ = Engine::GetInstance()->GetWinApp()->GetWindowSize();
    }
    if (_j.find("windowRatioSize") != _j.end()) {
        _j.at("windowRatioSize").get_to(_comp.windowRatioSize_);
        if (_comp.defaultWindowSize_.lengthSq() != 0.0f) {
            _comp.size_ = {_comp.defaultWindowSize_[X] * _comp.windowRatioSize_[X], _comp.defaultWindowSize_[Y] * _comp.windowRatioSize_[Y]};
        }
    } else {
        _comp.windowRatioSize_ = {_comp.size_[X] / _comp.defaultWindowSize_[X], _comp.size_[Y] / _comp.defaultWindowSize_[Y]};
    }
    if (_j.find("windowRatioPos") != _j.end()) {
        _j.at("windowRatioPos").get_to(_comp.windowRatioPos_);
        if (_comp.defaultWindowSize_.lengthSq() != 0.0f) {
            _comp.spriteBuff_->translate_ = {_comp.defaultWindowSize_[X] * _comp.windowRatioPos_[X], _comp.defaultWindowSize_[Y] * _comp.windowRatioPos_[Y]};
        }
    } else {
        _comp.windowRatioPos_ = {_comp.spriteBuff_->translate_[X] / _comp.defaultWindowSize_[X], _comp.spriteBuff_->translate_[Y] / _comp.defaultWindowSize_[Y]};
    }
}
