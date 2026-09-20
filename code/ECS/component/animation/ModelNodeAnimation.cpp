#include "ModelNodeAnimation.h"

/// engine
#define RESOURCE_DIRECTORY
#include "editor/EditorController.h"
#include "EngineInclude.h"
// asset
#include "asset/AssetSystem.h"
#include "asset/manager/AnimationAssetManager.h"

// assets
#include "model/Model.h"

#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef ORIGINE_EDITOR_ENABLED
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#include <imgui/imgui.h>

// Scene & Renderer (for Edit node tree)
#include "scene/Scene.h"
#include "component/renderer/ModelMeshRenderer.h"
#endif // _DEBUG

/// math
#include <cmath>

using namespace OriGine;

namespace {
/// <summary>
/// AnimationAssetManager を取得する（未登録なら nullptr）.
/// </summary>
AssetManager<AnimationAsset>* GetAnimationAssetManager() {
    return AssetSystem::GetInstance()->GetManager<AnimationAsset>();
}
} // namespace

/// <summary>
/// 参照中のアニメーションデータを取得する.
/// </summary>
AnimationData* ModelNodeAnimation::GetData() const {
    if (animationAssetIndex_ == kInvalidAssetIndex) {
        return nullptr;
    }
    auto* manager = GetAnimationAssetManager();
    if (manager == nullptr) {
        return nullptr;
    }
    AnimationAsset* asset = manager->GetMutableAsset(animationAssetIndex_);
    return asset ? &asset->data : nullptr;
}

/// <summary>
/// 参照中のアニメーションアセットを解放し、参照を無効化する.
/// </summary>
void ModelNodeAnimation::ReleaseAnimationAsset() {
    if (animationAssetIndex_ == kInvalidAssetIndex) {
        return;
    }
    if (auto* manager = GetAnimationAssetManager()) {
        manager->ReleaseAsset(animationAssetIndex_);
    }
    animationAssetIndex_ = kInvalidAssetIndex;
}

/// <summary>
/// directory_ / fileName_ に従ってアニメーションアセットを取得し直す.
/// </summary>
void ModelNodeAnimation::ReloadAnimationAsset() {
    ReleaseAnimationAsset();

    if (fileName_.empty()) {
        return;
    }
    auto* manager = GetAnimationAssetManager();
    if (manager == nullptr) {
        LOG_ERROR("AnimationAssetManager is not registered.");
        return;
    }
    animationAssetIndex_ = manager->LoadAsset(directory_ + "/" + fileName_);
}

/// <summary>
/// アニメーションデータが未参照であれば、空のアニメーションを新規登録して参照する.
/// </summary>
AnimationData* ModelNodeAnimation::GetOrCreateData() {
    if (AnimationData* data = GetData()) {
        return data;
    }

    auto* manager = GetAnimationAssetManager();
    if (manager == nullptr) {
        LOG_ERROR("AnimationAssetManager is not registered.");
        return nullptr;
    }

    // まだファイルを持たない新規アニメーション。
    // ファイルパスが決まっていない場合でも一意なキーが要るため、
    // 未保存であることが分かる論理パスを組み立てて登録する
    const std::string logicalPath = fileName_.empty()
                                        ? "memory://ModelNodeAnimation/" + std::to_string(reinterpret_cast<uintptr_t>(this))
                                        : directory_ + "/" + fileName_;

    animationAssetIndex_ = manager->RegisterAsset(logicalPath, AnimationAsset{});
    return GetData();
}

/// <summary>
/// 再生時刻をリセットし、必要ならアニメーションファイルを読み込む
/// </summary>
void ModelNodeAnimation::Initialize(Scene* /*_scene*/, const EntityHandle& /*_entity*/) {
    // 初期化
    currentAnimationTime_  = 0.0f;
    animationState_.isEnd_ = false;

    if (!fileName_.empty()) {
        ReloadAnimationAsset();
    }
}

void ModelNodeAnimation::Edit(Scene* _scene, const EntityHandle& _entity, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED
    std::string label = "Load File##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory, filename;
        if (MyFileSystem::SelectFileDialog(
                kApplicationResourceDirectory,
                directory,
                filename,
                {"gltf", "anm"})) {
            // コマンドを作成
            auto commandCombo = std::make_unique<CommandCombo>();

            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&fileName_, filename));
            commandCombo->SetFuncOnAfterCommand([this]() {
                ReloadAnimationAsset();

                if (const AnimationData* data = GetData()) {
                    duration_ = data->duration;
                }
            },
                true);

            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(("Save##" + _parentLabel).c_str())) {
        AnimationData* data = GetData();
        if (data && !fileName_.empty()) {
            data->duration = duration_;
            AnimationSerializer::Save(directory_, fileName_, *data);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(("Save As##" + _parentLabel).c_str())) {
        if (AnimationData* data = GetData()) {
            std::string directory, filename;
            if (MyFileSystem::SelectFileDialog(
                    kApplicationResourceDirectory,
                    directory,
                    filename,
                    {"anm"})) {
                directory_     = kApplicationResourceDirectory + "/" + directory;
                fileName_      = filename;
                data->duration = duration_;
                AnimationSerializer::Save(directory_, fileName_, *data);
            }
        }
    }

    ImGui::Text("File Name : %s", fileName_.c_str());

    label = "Duration##" + _parentLabel;
    DragGuiCommand(label, duration_, 0.01f, 0.0f);

    CheckBoxCommand("Is Loop##" + _parentLabel, animationState_.isLoop_);
    CheckBoxCommand("Is Play##" + _parentLabel, animationState_.isPlay_);

    // --- ModelMeshRenderer からノードツリーを取得・可視化 ---
    {
        ModelMeshData* modelData = nullptr;
        auto& renderers          = _scene->GetComponents<ModelMeshRenderer>(_entity);
        if (!renderers.empty()) {
            modelData = renderers[0].GetModelData();
        }

        if (modelData) {
            if (ImGui::TreeNode(("Node Tree##" + _parentLabel).c_str())) {
                // アニメーションデータがなければ空のものを新規登録して参照する
                AnimationData* data = GetOrCreateData();

                // 再帰的にノードツリーを表示
                std::function<void(const ModelNode&)> showNodeTree;
                showNodeTree = [&](const ModelNode& node) {
                    bool hasAnim = data->animationNodes_.count(node.name) > 0;

                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
                    if (node.children.empty()) {
                        flags |= ImGuiTreeNodeFlags_Leaf;
                    }

                    // アニメーションノードがあるノードは緑色で表示
                    if (hasAnim) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
                    }

                    bool opened = ImGui::TreeNodeEx(
                        (node.name + "##NodeTree" + _parentLabel).c_str(), flags);

                    if (hasAnim) {
                        ImGui::PopStyleColor();
                    }

                    // アニメーションノードがなければ追加ボタン
                    if (!hasAnim) {
                        ImGui::SameLine();
                        if (ImGui::SmallButton(("+##Add" + node.name + _parentLabel).c_str())) {
                            data->animationNodes_[node.name] = ModelAnimationNode();
                        }
                    } else {
                        // 削除ボタン
                        ImGui::SameLine();
                        if (ImGui::SmallButton(("-##Remove" + node.name + _parentLabel).c_str())) {
                            data->animationNodes_.erase(node.name);
                        }
                    }

                    if (opened) {
                        for (const auto& child : node.children) {
                            showNodeTree(child);
                        }
                        ImGui::TreePop();
                    }
                };

                if (data) {
                    showNodeTree(modelData->rootNode);
                }
                ImGui::TreePop();
            }
        }

        // 手動ノード追加
        {
            static char newNodeName[128] = "";
            ImGui::InputText(("New Node##Input" + _parentLabel).c_str(), newNodeName, sizeof(newNodeName));
            ImGui::SameLine();
            if (ImGui::Button(("Add##NewNode" + _parentLabel).c_str())) {
                if (strlen(newNodeName) > 0) {
                    // アニメーションデータがなければ空のものを新規登録して参照する
                    AnimationData* data = GetOrCreateData();
                    std::string name(newNodeName);
                    if (data && data->animationNodes_.find(name) == data->animationNodes_.end()) {
                        data->animationNodes_[name] = ModelAnimationNode();
                        newNodeName[0]              = '\0';
                    }
                }
            }
        }
    }

    ImGui::Separator();

    // ノードごとのキーフレーム編集
    AnimationData* editingData = GetData();
    if (editingData && !editingData->animationNodes_.empty()) {
        for (auto& [nodeName, nodeAnim] : editingData->animationNodes_) {
            if (!ImGui::TreeNode((nodeName + "##" + _parentLabel).c_str())) {
                continue;
            }

            // InterpolationType
            std::string comboLabel = "InterpolationType##" + nodeName + _parentLabel;
            if (ImGui::BeginCombo(comboLabel.c_str(), InterpolationTypeName[int(nodeAnim.interpolationType)])) {
                for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
                    if (ImGui::Selectable(InterpolationTypeName[i], nodeAnim.interpolationType == InterpolationType(i))) {
                        OriGine::EditorController::GetInstance()->PushCommand(
                            std::make_unique<SetterCommand<InterpolationType>>(&nodeAnim.interpolationType, InterpolationType(i)));
                    }
                }
                ImGui::EndCombo();
            }

            constexpr float kKeyFrameSliderWidth = 400.0f;

            // Scale
            ImGui::TextUnformatted("Scale");
            ImGui::SetNextItemWidth(kKeyFrameSliderWidth);
            ImGui::EditKeyFrame("##Scale" + nodeName + _parentLabel, nodeAnim.scale, duration_);

            ImGui::Separator();

            // Rotate
            ImGui::TextUnformatted("Rotate");
            ImGui::SetNextItemWidth(kKeyFrameSliderWidth);
            ImGui::EditKeyFrame("##Rotate" + nodeName + _parentLabel, nodeAnim.rotate, duration_);

            ImGui::Separator();

            // Translate
            ImGui::TextUnformatted("Translate");
            ImGui::SetNextItemWidth(kKeyFrameSliderWidth);
            ImGui::EditKeyFrame("##Translate" + nodeName + _parentLabel, nodeAnim.translate, duration_);

            ImGui::TreePop();
        }
    }

#endif // _DEBUG
}

void ModelNodeAnimation::Finalize() {
    // 参照カウントを戻しておかないと、コンポーネント破棄後もアセットが解放されない
    ReleaseAnimationAsset();
}

/// <summary>
/// 再生時刻を進め、モデルの全ノードにアニメーションを適用する
/// </summary>
/// <param name="_deltaTime">前フレームからの経過時間(秒)</param>
/// <param name="_model">対象モデル</param>
/// <param name="_parentTransform">ルートノードの親となるワールド行列</param>
void ModelNodeAnimation::UpdateModel(float _deltaTime, Model* _model, const Matrix4x4& _parentTransform) {
    {
        // isLoop_ が false の場合,一度終了したら return
        if (!animationState_.isLoop_) {
            if (animationState_.isEnd_) {
                return;
            }
        }

        animationState_.isEnd_ = false;
        // 時間更新
        currentAnimationTime_ += _deltaTime;

        // リピート
        // fmod で duration_ を超えた分を切り捨てず余りとして次周期の先頭からの経過時間に持ち越す
        // (単純に 0 へリセットすると、1フレームだけ長く経過したぶんの時間がロストしてしまう)
        // 注意: isLoop_ が true の場合でもここで isPlay_ が false になる。
        // 呼び出し側が IsPlay() を見て Update を呼ぶかどうかを判断しているなら、
        // ループ再生が一周目で止まって見える挙動になり得る点に留意する。
        if (currentAnimationTime_ > duration_) {
            animationState_.isEnd_  = true;
            animationState_.isPlay_ = false;
            currentAnimationTime_   = std::fmod(currentAnimationTime_, duration_);
        }
    }

    {
        ApplyAnimationToNodes(_model->meshData_->rootNode, _parentTransform, this);
    }
}

/// <summary>
/// Nodeアニメーションの現在のローカル行列を計算
/// </summary>
Matrix4x4 ModelNodeAnimation::CalculateNodeLocal(const std::string& _nodeName) const {
    const AnimationData* data = GetData();
    if (data == nullptr) {
        return MakeMatrix4x4::Identity();
    }

    auto it = data->animationNodes_.find(_nodeName);
    if (it == data->animationNodes_.end()) {
        // ノードに対応するアニメーションがない場合、単位行列を返す
        return MakeMatrix4x4::Identity();
    }

    const ModelAnimationNode& nodeAnimation = it->second;

    Vec3f scale;
    Quaternion rotate;
    Vec3f translate;

    // ノードごとに独立したキーフレーム配列を持つため、ノード単位で現在時刻の値を都度計算する。
    // LINEAR は内部で Quaternion 用に Slerp を使った補間になる。
    // Normalize は、Slerp の結果や連続する補間の積み重ねで生じる誤差により
    // クォータニオンの長さが 1 からずれ、行列化した際に意図しない拡縮が混ざるのを防ぐため
    switch (nodeAnimation.interpolationType) {
    case InterpolationType::LINEAR:
        scale     = CalculateValue::Linear(nodeAnimation.scale, currentAnimationTime_);
        rotate    = Quaternion::Normalize(CalculateValue::Linear(nodeAnimation.rotate, currentAnimationTime_));
        translate = CalculateValue::Linear(nodeAnimation.translate, currentAnimationTime_);
        break;
    case InterpolationType::STEP:
        scale     = CalculateValue::Step(nodeAnimation.scale, currentAnimationTime_);
        rotate    = Quaternion::Normalize(CalculateValue::Step(nodeAnimation.rotate, currentAnimationTime_));
        translate = CalculateValue::Step(nodeAnimation.translate, currentAnimationTime_);
        break;
    }

    return MakeMatrix4x4::Affine(scale, rotate, translate);
}

/// <summary>
/// ノードにアニメーションを適用
/// </summary>
void ModelNodeAnimation::ApplyAnimationToNodes(
    ModelNode& _node,
    const Matrix4x4& _parentTransform,
    const ModelNodeAnimation* _animation) {
    _node.localMatrix         = _animation->CalculateNodeLocal(_node.name);
    // 親のワールド行列 × 自身のローカル行列で、このノードのワールド行列を求める。
    // その結果を子ノードへ渡すことで、ルートから葉ノードへ向けて変換が順に伝播していく
    Matrix4x4 globalTransform = _parentTransform * _node.localMatrix;

    // 子ノードに再帰的に適用
    for (auto& child : _node.children) {
        ApplyAnimationToNodes(child, globalTransform, _animation);
    }
}

/// <summary>
/// 指定ノードの現在時刻でのスケール値を取得(アニメーションが無ければ等倍)
/// </summary>
Vec3f ModelNodeAnimation::GetCurrentScale(const std::string& _nodeName) const {
    const AnimationData* data = GetData();
    if (data == nullptr) {
        return Vec3f(1.0f, 1.0f, 1.0f);
    }

    auto itr = data->animationNodes_.find(_nodeName);
    if (itr == data->animationNodes_.end()) {
        return Vec3f(1.0f, 1.0f, 1.0f);
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.scale, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.scale, currentAnimationTime_);
}

/// <summary>
/// 指定ノードの現在時刻での回転値を取得(アニメーションが無ければ単位クォータニオン)
/// </summary>
Quaternion ModelNodeAnimation::GetCurrentRotate(const std::string& _nodeName) const {
    const AnimationData* data = GetData();
    if (data == nullptr) {
        return Quaternion::Identity();
    }

    auto itr = data->animationNodes_.find(_nodeName);
    if (itr == data->animationNodes_.end()) {
        return Quaternion::Identity();
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.rotate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.rotate, currentAnimationTime_);
}

/// <summary>
/// 指定ノードの現在時刻での平行移動値を取得(アニメーションが無ければゼロベクトル)
/// </summary>
Vec3f ModelNodeAnimation::GetCurrentTranslate(const std::string& _nodeName) const {
    const AnimationData* data = GetData();
    if (data == nullptr) {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }

    auto itr = data->animationNodes_.find(_nodeName);
    if (itr == data->animationNodes_.end()) {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }
    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.translate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.translate, currentAnimationTime_);
}

/// <summary>
/// ModelNodeAnimation を JSON へ書き出す
/// </summary>
void OriGine::to_json(nlohmann::json& _j, const ModelNodeAnimation& _comp) {
    _j = nlohmann::json{
        {"directory", _comp.directory_},
        {"fileName", _comp.fileName_},
        {"isPlay", _comp.animationState_.isPlay_},
        {"isLoop", _comp.animationState_.isLoop_},
        {"duration", _comp.duration_},
        {"currentAnimationTime", _comp.currentAnimationTime_}};
}

/// <summary>
/// JSON から ModelNodeAnimation を復元する
/// </summary>
void OriGine::from_json(const nlohmann::json& _j, ModelNodeAnimation& _comp) {
    _j.at("directory").get_to(_comp.directory_);
    _j.at("fileName").get_to(_comp.fileName_);
    _j.at("isPlay").get_to(_comp.animationState_.isPlay_);
    _j.at("isLoop").get_to(_comp.animationState_.isLoop_);
    _j.at("duration").get_to(_comp.duration_);
    _j.at("currentAnimationTime").get_to(_comp.currentAnimationTime_);
}
