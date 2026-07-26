/**
 * @file LineRenderer.cpp
 * @brief LineRendererコンポーネントの実装
 *
 * ラインレンダラーの全機能（コア、エディタ、シリアライズ）を含む。
 */
#include "LineRenderer.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"

// directX12
#include "directX12/DxDevice.h"

#ifdef _DEBUG
/// editor
#include "editor/EditorController.h"

/// util
#include "util/myGui/MyGui.h"

/// externals
#include <imgui/imgui.h>
#endif // _DEBUG

/// externals
#include <nlohmann/json.hpp>

using namespace OriGine;

//==============================================================================
// LineRenderer - Core Implementation
//==============================================================================

LineRenderer::LineRenderer() : MeshRenderer() {
    meshGroup_->push_back(Mesh<ColorVertexData>());
    auto& mesh = meshGroup_->back();
    // 頂点/インデックスバッファを100個分の容量で確保しておき(頻繁な再確保を避けるための予約)、
    // 実際に使用中のサイズは0から始めてラインが追加されるたびに増やしていく
    mesh.Initialize(100, 100);
    mesh.SetVertexSize(0);
    mesh.SetIndexSize(0);
}

LineRenderer::LineRenderer(const std::vector<Mesh<ColorVertexData>>& _meshGroup)
    : MeshRenderer<Mesh<ColorVertexData>, ColorVertexData>(_meshGroup) {}

LineRenderer::LineRenderer(const std::shared_ptr<std::vector<Mesh<ColorVertexData>>>& _meshGroup)
    : MeshRenderer<Mesh<ColorVertexData>, ColorVertexData>(_meshGroup) {}

LineRenderer::~LineRenderer() {}

void LineRenderer::Initialize(Scene* _scene, const EntityHandle& _hostEntity) {
    MeshRenderer::Initialize(_scene, _hostEntity);
    // GPU上に定数バッファリソースを確保し、Map済みポインタを保持する(以降はMap/Unmapし直さず書き込むだけで済む)
    transformBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    transformBuff_.openData_.UpdateMatrix();
    // CPU側の行列データ(openData_)をMap済みのGPUメモリへコピーする。この呼び出しをしない限りGPU側には反映されない
    transformBuff_.ConvertToBuffer();
}

void LineRenderer::Finalize() {
    if (meshGroup_) {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
    }
    meshGroup_.reset();
    transformBuff_.Finalize();
}

//==============================================================================
// LineRenderer - Editor (Debug Only)
//==============================================================================

#ifdef _DEBUG
/// <summary>
/// LineRendererにラインを追加するコマンド
/// </summary>
class AddLineCommand
    : public IEditCommand {
public:
    AddLineCommand(std::shared_ptr<std::vector<Mesh<ColorVertexData>>>& _meshGroup)
        : meshGroup_(_meshGroup), addedMeshIndex_(-1), vertex1Index_(-1), vertex2Index_(-1) {}

    /// <summary>
    /// ラインを1本追加する。末尾メッシュの空き容量が無ければ新しいメッシュを追加する
    /// </summary>
    void Execute() override {
        Mesh<ColorVertexData>* mesh = &meshGroup_->back();

        // 末尾メッシュの予約容量(Initializeで確保した100個分)を使い切っている場合は
        // 新しいメッシュを追加してそちらにラインを積んでいく
        if (mesh->GetIndexCapacity() - mesh->GetIndexSize() <= 0) {
            meshGroup_->emplace_back();
            mesh = &meshGroup_->back();
            mesh->Initialize(100, 100);

            mesh->SetVertexSize(2);
            mesh->SetIndexSize(2);

            addedMeshIndex_ = static_cast<int32_t>(meshGroup_->size() - 1);
        } else {
            mesh->SetVertexSize(mesh->GetVertexSize() + 2);
            mesh->SetIndexSize(mesh->GetIndexSize() + 2);
        }

        // 頂点の初期化
        vertex1Index_ = mesh->GetVertexSize() - 2;
        vertex2Index_ = mesh->GetVertexSize() - 1;

        mesh->vertexes_[vertex1Index_].pos = Vector4(0.0f, 0.0f, 0.0f, 0.f);
        mesh->vertexes_[vertex2Index_].pos = Vector4(0.0f, 0.0f, 0.0f, 0.f);

        mesh->vertexes_[vertex1Index_].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        mesh->vertexes_[vertex2Index_].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

        mesh->indexes_[vertex1Index_] = vertex1Index_;
        mesh->indexes_[vertex2Index_] = vertex2Index_;
    }

    /// <summary>
    /// Execute()で追加したラインを取り消す
    /// </summary>
    void Undo() override {
        if (addedMeshIndex_ != -1) {
            // Execute()で新しいメッシュを追加していた場合は、それをそのまま末尾から取り除くだけで元に戻せる
            // (常に末尾への追加のみを行うため、popによる取り消しでインデックスがずれる心配はない)
            meshGroup_->pop_back();
        } else {
            Mesh<ColorVertexData>* mesh = &meshGroup_->back();
            mesh->SetVertexSize(mesh->GetVertexSize() - 2);
            mesh->SetIndexSize(mesh->GetIndexSize() - 2);
        }
    }

private:
    std::shared_ptr<std::vector<Mesh<ColorVertexData>>> meshGroup_;
    int32_t addedMeshIndex_; // 新しく追加したメッシュのインデックス
    int32_t vertex1Index_; // 追加した頂点1のインデックス
    int32_t vertex2Index_; // 追加した頂点2のインデックス
};
#endif // _DEBUG

void LineRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    std::string label;
    int32_t meshIndex = 0;
    int32_t lineIndex = 0;

    std::string startLabel;
    std::string endLabel;
    std::string startColorLabel;
    std::string endColorLabel;

    ImGui::Checkbox("LineIsStrip", &lineIsStrip_);

    transformBuff_.openData_.Edit(_scene, _handle, _parentLabel);

    for (auto& mesh : *meshGroup_) {
        label = "Mesh[" + std::to_string(meshIndex) + "]";
        if (ImGui::TreeNode(label.c_str())) {

            for (size_t i = 0; i < mesh.vertexes_.size(); i += 2) {
                // 2つの頂点を更新
                auto& vertex1 = mesh.vertexes_[i];
                auto& vertex2 = mesh.vertexes_[i + 1];

                std::string lineLabel = "Line[" + std::to_string(lineIndex) + "]";
                ImGui::Text("%s", lineLabel.c_str());

                startLabel      = "start##" + std::to_string(lineIndex);
                endLabel        = "end##" + std::to_string(lineIndex);
                startColorLabel = "start Color##" + std::to_string(lineIndex);
                endColorLabel   = "end Color##" + std::to_string(lineIndex);

                DragGuiVectorCommand(startLabel.c_str(), vertex1.pos);
                vertex1.pos[W] = 1.f;
                DragGuiVectorCommand(endLabel.c_str(), vertex2.pos);
                vertex2.pos[W] = 1.f;
                ImGui::Spacing();
                ColorEditGuiCommand(startColorLabel, vertex1.color);
                ColorEditGuiCommand(endColorLabel, vertex2.color);

                ++lineIndex;
            }
            ImGui::TreePop();

            // ImGuiは毎フレーム再描画される即時モードGUIのため、TreeNodeが開いている間は
            // 変更の有無に関わらず毎フレームここを通る。エディタ専用パス(_DEBUGビルドのみ)で
            // メッシュサイズも小さいため、都度TransferData()でGPU側頂点バッファへ反映しても問題にならない
            mesh.TransferData();
            ++meshIndex;
        }
    }

    if (ImGui::Button("AddLine")) {
        auto command = std::make_unique<AddLineCommand>(meshGroup_);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

#endif // _DEBUG
}

//==============================================================================
// LineRenderer - JSON Serialization
//==============================================================================

namespace OriGine {

/// <summary>
/// LineRendererの状態をjsonへ書き出す。GPU側バッファではなくCPU側ミラー(openData_やvertexes_/indexes_)を書き出す
/// </summary>
void to_json(nlohmann::json& _j, const LineRenderer& _comp) {
    _j["isRender"]  = _comp.isRender_;
    _j["blendMode"] = static_cast<int32_t>(_comp.currentBlend_);

    _j["lineIsStrip"] = _comp.lineIsStrip_;

    // transform
    nlohmann::json transformBufferData;
    to_json(transformBufferData, _comp.transformBuff_.openData_);
    _j["transformBufferData"] = transformBufferData;

    // mesh
    nlohmann::json meshGroupDatas = nlohmann::json::array();
    for (uint32_t meshIndex = 0; meshIndex < _comp.meshGroup_->size(); ++meshIndex) {
        nlohmann::json meshData;

        meshData["vertexSize"] = _comp.meshGroup_->at(meshIndex).GetVertexSize();
        meshData["indexSize"]  = _comp.meshGroup_->at(meshIndex).GetIndexSize();

        meshData["vertexes"] = nlohmann::json::array();
        for (uint32_t vertexIndex = 0; vertexIndex < _comp.meshGroup_->at(meshIndex).GetVertexSize(); ++vertexIndex) {
            nlohmann::json vertexData;
            vertexData["pos"]   = _comp.meshGroup_->at(meshIndex).vertexes_[vertexIndex].pos;
            vertexData["color"] = _comp.meshGroup_->at(meshIndex).vertexes_[vertexIndex].color;

            meshData["vertexes"].emplace_back(vertexData);
        }

        meshData["indexes"] = nlohmann::json::array();
        for (uint32_t indexIndex = 0; indexIndex < _comp.meshGroup_->at(meshIndex).GetIndexSize(); ++indexIndex) {
            nlohmann::json indexData;
            indexData = _comp.meshGroup_->at(meshIndex).indexes_[indexIndex];
            meshData["indexes"].emplace_back(indexData);
        }

        meshGroupDatas.emplace_back(meshData);
    }

    _j["meshGroupDatas"] = meshGroupDatas;
}

/// <summary>
/// jsonからLineRendererの状態を復元する。
/// meshGroup_の各要素をemplace_backで積み増していく(既存要素をclearしていない点に注意)
/// </summary>
void from_json(const nlohmann::json& _j, LineRenderer& _comp) {
    _j.at("isRender").get_to(_comp.isRender_);
    int32_t blendMode = 0;
    _j.at("blendMode").get_to(blendMode);
    _comp.currentBlend_ = static_cast<BlendMode>(blendMode);

    _j.at("lineIsStrip").get_to(_comp.lineIsStrip_);

    // transform
    _j.at("transformBufferData").get_to(_comp.transformBuff_.openData_);

    // mesh
    auto& meshGroupDatas = _j.at("meshGroupDatas");
    for (auto& meshData : meshGroupDatas) {
        Mesh<ColorVertexData> mesh;
        mesh.SetVertexSize(meshData.at("vertexSize"));
        mesh.SetIndexSize(meshData.at("indexSize"));
        for (auto& vertexData : meshData.at("vertexes")) {
            ColorVertexData vertex;
            vertex.pos   = vertexData.at("pos");
            vertex.color = vertexData.at("color");
            mesh.vertexes_.emplace_back(vertex);
        }
        for (auto& indexData : meshData.at("indexes")) {
            int32_t index;
            index = indexData;
            mesh.indexes_.emplace_back(index);
        }
        _comp.meshGroup_->emplace_back(mesh);
    }
}

} // namespace OriGine
