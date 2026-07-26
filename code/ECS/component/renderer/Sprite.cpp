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

/// <summary>
/// スプライト描画に必要なGPUリソース（定数バッファ・矩形メッシュ）を構築し、
/// 設定済みのテクスチャがあれば読み込んで表示サイズを決定する
/// </summary>
/// <param name="_scene">所属シーン</param>
/// <param name="_hostEntity">このコンポーネントを保持するエンティティ</param>
void SpriteRenderer::Initialize(Scene* _scene, const EntityHandle& _hostEntity) {
    MeshRenderer::Initialize(_scene, _hostEntity);

    // buffer作成(定数バッファ用のGPUリソースを確保し、Map済みポインタを保持する)
    spriteBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    // メッシュの初期化
    meshGroup_ = std::make_shared<std::vector<SpriteMesh>>();
    meshGroup_->push_back(SpriteMesh());

    SpriteMesh& mesh = meshGroup_->at(0);
    // スプライトは矩形1枚(頂点4個・三角形2枚=インデックス6個)だけで表現できる
    mesh.Initialize(4, 6);
    // indexData
    // 頂点0,1,2,3を左下/左上/右下/右上として、(0,1,2)と(1,3,2)の2枚の三角形で矩形を構成する
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

/// <summary>
/// デバッグ用GUIでスプライトのパラメータを編集する。
/// px値と比率のどちらを編集しても、編集後コールバックでもう一方を再計算して同期させる
/// </summary>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void SpriteRenderer::Edit(Scene* /*_scene*/, const EntityHandle& /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    // エディタ専用のUIコードなので、リリースビルドには含めない

    // px値を編集したときは比率側を、比率を編集したときはpx値側を再計算する
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

/// <summary>
/// 終了処理。基底が持つメッシュ資源を解放したあと、
/// このクラス固有の定数バッファ（Map済みGPUメモリ）も解放する
/// </summary>
void SpriteRenderer::Finalize() {
    MeshRenderer::Finalize();
    spriteBuff_.Finalize();
}

/// <summary>
/// 画面サイズに対する比率(0~1)を正として、px単位の絶対位置・サイズを再計算する。
///
/// SpriteRendererは「px単位の絶対値」と「画面サイズに対する比率」の2種類の値を併せ持つ。
/// 比率側を正とするのがこの関数、絶対値側を正として比率を求め直すのが
/// CalculatePosRatioAndSizeRatio で、両者は逆方向の変換になっている。
/// 解像度が変わっても見た目の配置比率を保ちたいUI用途で、どちらから編集されても
/// 両方の値を同期できるようにするための仕組み。
///
/// lengthSq() が0のときに代入をスキップしているのは、比率が未設定(ゼロ初期化のまま)の
/// スプライトの絶対値を0で潰してしまわないため
/// </summary>
void SpriteRenderer::CalculateWindowRatioPosAndSize() {
    if (windowRatioSize_.lengthSq() != 0.0f) {
        size_ = {defaultWindowSize_[X] * windowRatioSize_[X], defaultWindowSize_[Y] * windowRatioSize_[Y]};
    }
    if (windowRatioPos_.lengthSq() != 0.0f) {
        spriteBuff_->translate_ = {defaultWindowSize_[X] * windowRatioPos_[X], defaultWindowSize_[Y] * windowRatioPos_[Y]};
    }
}

/// <summary>
/// 基準となる画面サイズを更新したうえで、比率からpx単位の位置・サイズを再計算する。
/// ウィンドウリサイズ通知を受けた際に呼ばれる
/// </summary>
/// <param name="_newWindowSize">新しい画面サイズ[px]</param>
void SpriteRenderer::CalculateWindowRatioPosAndSize(const Vec2f& _newWindowSize) {
    defaultWindowSize_ = _newWindowSize;
    CalculateWindowRatioPosAndSize();
}

/// <summary>
/// px単位の絶対位置・サイズを正として、画面サイズに対する比率を再計算する。
/// エディタ上でpx値を直接編集した後に、比率側の値を追従させる用途で使う
/// </summary>
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

/// <summary>
/// 基準となる画面サイズを更新したうえで、px値から比率を再計算する
/// </summary>
/// <param name="_newWindowSize">新しい画面サイズ[px]</param>
void SpriteRenderer::CalculatePosRatioAndSizeRatio(const Vec2f& _newWindowSize) {
    defaultWindowSize_ = _newWindowSize;
    CalculatePosRatioAndSizeRatio();
}

/// <summary>
/// 表示するテクスチャを差し替える
/// </summary>
/// <param name="_texturePath">読み込むテクスチャのパス</param>
/// <param name="_applyTextureSize">
/// trueなら、読み込んだテクスチャの実サイズをスプライトの既定サイズとして採用する。
/// ただし既に textureSize_ / size_ が設定済み（lengthSqが0でない）の場合は上書きしない。
/// これは、エディタやJSONで明示的に指定した切り出し範囲・表示サイズを、
/// テクスチャ差し替えのたびに実サイズで潰してしまわないため
/// </param>
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

/// <summary>
/// CPU側で保持しているスプライトの状態をGPUバッファへ転送する。
///
/// 毎フレーム描画システムから呼ばれ、ワールド行列/UV行列(定数バッファ)と
/// 頂点位置・UV座標(頂点バッファ)の両方を更新する。
/// つまりGPUへの反映タイミングは「フレームごとにUpdateBufferが呼ばれたとき」であり、
/// Setter(SetScale等)を呼んだ直後に即座に反映されるわけではない点に注意
/// </summary>
/// <param name="_viewPortMat">スクリーン座標をクリップ空間へ変換するビューポート行列</param>
void SpriteRenderer::UpdateBuffer(const Matrix4x4& _viewPortMat) {
    //-------------------------------- ConstBufferの更新 --------------------------------//
    {
        // スケール/回転/移動からワールド行列を再計算し、CPU側(openData_)を更新する
        spriteBuff_->Update(_viewPortMat);

        // 更新したCPU側の値をMap済みのGPUメモリへコピーする
        spriteBuff_.ConvertToBuffer();
    }
    //-------------------------------- メッシュの更新 --------------------------------//
    // anchorPoint_(0~1)を基準に、原点からの相対位置として頂点の四辺を求める
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

    // 位置・UV座標を書き換えたCPU側の頂点データをGPU側の頂点バッファへ転送する
    mesh.TransferData();
}

/// <summary>
/// SpriteRendererの状態をjsonへ書き出す。矩形形状(meshGroup_)は固定のため保存せず、
/// テクスチャパスやTransform/UV情報など再構築に必要なパラメータのみ保存する
/// </summary>
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

/// <summary>
/// jsonからSpriteRendererの状態を復元する。
/// defaultWindowSize/windowRatioSize/windowRatioPosは後から追加されたフィールドのため、
/// 保存データに存在しない場合は絶対値側(size_/translate_)から比率側を逆算して補う(後方互換のため)
/// </summary>
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
