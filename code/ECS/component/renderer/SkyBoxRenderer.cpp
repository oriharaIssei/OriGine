#include "SkyboxRenderer.h"

/// engine
#define ENGINE_ECS
#define ENGINE_EDITOR
#define RESOURCE_DIRECTORY
#include "asset/AssetSystem.h"
#include "EngineInclude.h"
// asset
#include "asset/TextureAsset.h"
// directX12
#include "directX12/DxDevice.h"

#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef ORIGINE_EDITOR_ENABLED
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SkyboxRenderer::Initialize(Scene* _scene, const EntityHandle& _hostEntity) {
    MeshRenderer::Initialize(_scene, _hostEntity);

    isRender_ = true;

    /// mesh
    meshGroup_->push_back(Mesh<SkyboxVertex>());
    auto& mesh = meshGroup_->back();
    // 立方体は頂点8個・三角形12枚(インデックス36個)で表現できるため、その固定サイズで確保する
    mesh.Initialize(8, 36);

    // 前
    mesh.vertexes_[0].position = {-1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[1].position = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[2].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[3].position = {1.f, -1.f, 1.f, 1.f};
    // 後ろ
    mesh.vertexes_[4].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[5].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[6].position = {-1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[7].position = {1.f, -1.f, -1.f, 1.f};

    // 天空箱はカメラが立方体の内側に入り込んで内壁を見る形になるため、
    // 各面のインデックス順は、通常の外向き立方体とは反対に「内側から見て」表となる巻き順で並べている
    mesh.indexes_ = {
        // 前
        0,
        1,
        2,
        2,
        1,
        3,
        // 後ろ
        4,
        6,
        5,
        7,
        5,
        6,
        // 右
        1,
        5,
        3,
        3,
        5,
        7,
        // 左
        0,
        2,
        4,
        6,
        4,
        2,
        // 上
        4,
        5,
        0,
        0,
        5,
        1,
        // 下
        2,
        3,
        6,
        7,
        6,
        3,
    };

    // 固定形状の頂点/インデックスデータをGPU側の頂点・インデックスバッファへ転送する(以降変化しない)
    mesh.TransferData();

    if (!filePath_.empty()) {
        // シリアライズ済みのファイルパスがあれば、そのキューブマップテクスチャを読み込む
        textureIndex_ = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(filePath_);
    }

    transformBuff_->Initialize(_scene, _hostEntity);
    // Transform/Material用の定数バッファのGPUリソースを確保する
    transformBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    materialBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void SkyboxRenderer::Edit(Scene* /*_scene*/, const EntityHandle& /* _entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED

    ImGui::Text("FilePath : %s", filePath_.c_str());
    std::string label = "Load##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory;
        std::string filename;
        myfs::SelectFileDialog(kApplicationResourceDirectory, directory, filename, {"dds"});
        if (!filename.empty()) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->AddCommand(std::make_unique<SetterCommand<std::string>>(&filePath_, kApplicationResourceDirectory + "/" + directory + "/" + filename));
            commandCombo->SetFuncOnAfterCommand(
                [this]() {
                    textureIndex_ =
                        AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(filePath_);
                },
                true);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    ColorEditGuiCommand("Color##" + _parentLabel, materialBuff_.openData_.color);
#endif // _DEBUG
}

/// <summary>
/// SkyboxRendererの状態をjsonへ書き出す。立方体形状は固定なのでmeshGroup_は保存しない
/// </summary>
void OriGine::to_json(nlohmann::json& _j, const SkyboxRenderer& _comp) {
    _j["filePath"]      = _comp.filePath_;
    _j["transformBuff"] = _comp.transformBuff_.openData_;
    _j["materialBuff"]  = _comp.materialBuff_.openData_;
}

/// <summary>
/// jsonからSkyboxRendererの状態を復元する
/// </summary>
void OriGine::from_json(const nlohmann::json& _j, SkyboxRenderer& _comp) {
    _j.at("filePath").get_to(_comp.filePath_);
    _j.at("transformBuff").get_to(_comp.transformBuff_.openData_);
    _j.at("materialBuff").get_to(_comp.materialBuff_.openData_);
}

/// <summary>
/// SkyboxMaterialの状態をjsonへ書き出す
/// </summary>
void OriGine::to_json(nlohmann::json& _j, const SkyboxMaterial& _comp) {
    _j["color"] = _comp.color;
}

/// <summary>
/// jsonからSkyboxMaterialの状態を復元する
/// </summary>
void OriGine::from_json(const nlohmann::json& _j, SkyboxMaterial& _comp) {
    _j.at("color").get_to(_comp.color);
}
