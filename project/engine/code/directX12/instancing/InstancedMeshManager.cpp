#include "InstancedMeshManager.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"
// directX12
#include "directX12/DxDevice.h"
// asset
#include "asset/AssetSystem.h"
#include "asset/TextureAsset.h"
// model
#include "model/ModelManager.h"
// logger
#include "logger/Logger.h"

using namespace OriGine;

//==============================================================================
// InstancedMeshManager - Internal Helpers
//==============================================================================

namespace {

/// <summary>
/// ModelNode を再帰走査し、meshGroup にマッチするサブメッシュと
/// そのローカル Transform を収集する.
/// CreateModelMeshRenderer と同じ走査ロジック.
/// </summary>
void CollectSubmeshes(
    ModelMeshData* _modelData,
    ModelNode* _node,
    std::vector<TextureColorMesh*>& _outSubmeshOrder,
    std::vector<Transform>& _outLocalTransforms) {

    auto meshItr = _modelData->meshGroup.find(_node->name);
    if (meshItr != _modelData->meshGroup.end()) {
        _outSubmeshOrder.push_back(&meshItr->second);
        _outLocalTransforms.push_back(_node->transform);
    }
    for (auto& child : _node->children) {
        CollectSubmeshes(_modelData, &child, _outSubmeshOrder, _outLocalTransforms);
    }
}

} // namespace

//==============================================================================
// InstancedMeshManager - Core Implementation
//==============================================================================

void InstancedMeshManager::Initialize() {
    // 現時点では特別な初期化は不要
}

void InstancedMeshManager::Finalize() {
    for (auto& [key, group] : modelGroups_) {
        for (auto& buffer : group.transformBuffers) {
            buffer.Finalize();
        }
        for (auto& buffer : group.materialBuffers) {
            buffer.Finalize();
        }
        group.instances.Clear();
    }
    modelGroups_.clear();

    // プリミティブテンプレートメッシュのクリーンアップ
    for (auto& [name, meshData] : primitiveTemplates_) {
        for (auto& [meshName, mesh] : meshData->meshGroup) {
            mesh.Finalize();
        }
    }
    primitiveTemplates_.clear();
}

InstanceHandle InstancedMeshManager::AddInstance(ModelMeshData* _modelData, EntityHandle _owner) {
    InstanceHandle handle;
    handle.group = _modelData;

    // グループが存在しなければ新規作成
    auto itr = modelGroups_.find(_modelData);
    if (itr == modelGroups_.end()) {
        InstancedModelGroup newGroup;
        newGroup.modelData = _modelData;
        BuildSubmeshOrder(newGroup);

        // submesh 毎に StructuredBuffer を作成
        auto device             = Engine::GetInstance()->GetDxDevice()->device_;
        uint32_t submeshCount   = static_cast<uint32_t>(newGroup.submeshOrder.size());

        newGroup.transformBuffers.resize(submeshCount);
        newGroup.materialBuffers.resize(submeshCount);
        for (uint32_t i = 0; i < submeshCount; ++i) {
            newGroup.transformBuffers[i].CreateBuffer(device, kDefaultInstanceCapacity);
            newGroup.materialBuffers[i].CreateBuffer(device, kDefaultInstanceCapacity);
        }

        itr = modelGroups_.emplace(_modelData, std::move(newGroup)).first;
    }

    auto& group = itr->second;

    // InstanceEntry を構築
    InstanceEntry entry;
    entry.owner = _owner;

    // デフォルトマテリアル・テクスチャで初期化
    const auto& defaultMaterials = ModelManager::GetInstance()->GetDefaultMaterials(_modelData);
    uint32_t submeshCount        = static_cast<uint32_t>(group.submeshOrder.size());
    entry.materials.resize(submeshCount);
    entry.textureIndices.resize(submeshCount);
    entry.localTransforms.resize(submeshCount);
    entry.customTextureHandles.resize(submeshCount, D3D12_GPU_DESCRIPTOR_HANDLE{0});

    for (uint32_t i = 0; i < submeshCount; ++i) {
        if (i < defaultMaterials.size()) {
            entry.materials[i]     = defaultMaterials[i].material.openData_;
            entry.textureIndices[i] = defaultMaterials[i].textureIndex;
        }
        // ローカル Transform はグループのデフォルトで初期化
        if (i < group.submeshLocalTransforms.size()) {
            entry.localTransforms[i] = group.submeshLocalTransforms[i];
        }
    }

    // DenseSlotMap に追加
    handle.id = group.instances.Insert(std::move(entry));

    return handle;
}

void InstancedMeshManager::RemoveInstance(const InstanceHandle& _handle) {
    if (!IsValid(_handle)) {
        LOG_WARN("InstancedMeshManager::RemoveInstance - Invalid handle.");
        return;
    }

    auto itr = modelGroups_.find(_handle.group);
    if (itr != modelGroups_.end()) {
        itr->second.instances.Erase(_handle.id);

        // グループが空になったらバッファを解放して削除
        if (itr->second.instances.Empty()) {
            for (auto& buffer : itr->second.transformBuffers) {
                buffer.Finalize();
            }
            for (auto& buffer : itr->second.materialBuffers) {
                buffer.Finalize();
            }
            modelGroups_.erase(itr);
        }
    }
}

InstanceEntry& InstancedMeshManager::GetInstance(const InstanceHandle& _handle) {
    return modelGroups_.at(_handle.group).instances.At(_handle.id);
}

bool InstancedMeshManager::IsValid(const InstanceHandle& _handle) const {
    if (!_handle.IsValid()) {
        return false;
    }
    auto itr = modelGroups_.find(_handle.group);
    if (itr == modelGroups_.end()) {
        return false;
    }
    return itr->second.instances.IsValid(_handle.id);
}

bool InstancedMeshManager::IsEmpty() const {
    return modelGroups_.empty();
}

void InstancedMeshManager::UpdateBuffers(Scene* _scene) {
    if (modelGroups_.empty()) {
        return;
    }

    for (auto& [key, group] : modelGroups_) {
        if (group.instances.Empty()) {
            continue;
        }

        uint32_t instanceCount = static_cast<uint32_t>(group.instances.Size());
        uint32_t submeshCount  = static_cast<uint32_t>(group.submeshOrder.size());

        // 容量チェック
        EnsureCapacity(group, instanceCount);

        // テクスチャバッチ情報をリサイズ
        group.textureBatches.resize(submeshCount);

        // ソート用の一時データ
        struct InstanceSortEntry {
            uint32_t originalIndex;
            D3D12_GPU_DESCRIPTOR_HANDLE textureHandle;
            BlendMode blendMode;
            bool isCulling;
        };

        auto* texAssetMgr = AssetSystem::GetInstance()->GetManager<TextureAsset>();

        for (uint32_t j = 0; j < submeshCount; ++j) {
            auto& transformBuffer = group.transformBuffers[j];
            auto& materialBuffer  = group.materialBuffers[j];

            transformBuffer.openData_.clear();
            materialBuffer.openData_.clear();
            transformBuffer.openData_.reserve(instanceCount);
            materialBuffer.openData_.reserve(instanceCount);

            // ソート用インデックスを構築
            std::vector<InstanceSortEntry> sortEntries;
            sortEntries.reserve(instanceCount);
            for (uint32_t i = 0; i < instanceCount; ++i) {
                auto& entry = group.instances.DataAt(i);

                // customTextureHandles があればそれを使用、なければ textureIndex から解決
                D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
                if (j < entry.customTextureHandles.size() && entry.customTextureHandles[j].ptr != 0) {
                    handle = entry.customTextureHandles[j];
                } else {
                    size_t texIdx = (j < entry.textureIndices.size()) ? entry.textureIndices[j] : 0;
                    handle = texAssetMgr->GetAsset(texIdx).srv.GetGpuHandle();
                }
                sortEntries.push_back({i, handle, entry.blendMode, entry.isCulling});
            }

            // (カリング, ブレンドモード, テクスチャ) の順でソート
            std::sort(sortEntries.begin(), sortEntries.end(),
                [](const InstanceSortEntry& a, const InstanceSortEntry& b) {
                    if (a.isCulling != b.isCulling)     return a.isCulling > b.isCulling; // culling=true を先に
                    if (a.blendMode != b.blendMode)     return a.blendMode < b.blendMode;
                    return a.textureHandle.ptr < b.textureHandle.ptr;
                });

            // ソート順に Transform / Material を構築 & バッチを記録
            auto& batches = group.textureBatches[j];
            batches.clear();

            for (uint32_t sortIdx = 0; sortIdx < instanceCount; ++sortIdx) {
                auto& sortEntry = sortEntries[sortIdx];
                auto& entry = group.instances.DataAt(sortEntry.originalIndex);

                // エンティティの Transform を取得
                Transform* entityTransform = _scene->GetComponent<Transform>(entry.owner);
                if (!entityTransform) {
                    Transform identity;
                    identity.UpdateMatrix();
                    transformBuffer.openData_.push_back(identity);
                    materialBuffer.openData_.emplace_back();
                } else {
                    // インスタンス固有のローカル Transform を使用
                    Transform combined;
                    if (j < entry.localTransforms.size()) {
                        combined = entry.localTransforms[j];
                    } else {
                        combined = group.submeshLocalTransforms[j];
                    }
                    combined.parent = entityTransform;
                    combined.UpdateMatrix();
                    transformBuffer.openData_.push_back(combined);

                    // マテリアル
                    if (j < entry.materials.size()) {
                        Material mat = entry.materials[j];
                        mat.UpdateUvMatrix();
                        materialBuffer.openData_.push_back(mat);
                    } else {
                        materialBuffer.openData_.emplace_back();
                    }
                }

                // バッチの記録（カリング・ブレンドモード・テクスチャが同一のものをまとめる）
                bool sameBatch = !batches.empty()
                    && batches.back().textureHandle.ptr == sortEntry.textureHandle.ptr
                    && batches.back().blendMode == sortEntry.blendMode
                    && batches.back().isCulling == sortEntry.isCulling;

                if (sameBatch) {
                    batches.back().instanceCount++;
                } else {
                    TextureBatch batch;
                    batch.textureHandle = sortEntry.textureHandle;
                    batch.startInstance = sortIdx;
                    batch.instanceCount = 1;
                    batch.blendMode     = sortEntry.blendMode;
                    batch.isCulling     = sortEntry.isCulling;
                    batches.push_back(batch);
                }
            }

            transformBuffer.ConvertToBuffer();
            materialBuffer.ConvertToBuffer();
        }
    }

    RebuildInstanceFlags();
}

void InstancedMeshManager::Render(
    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _cmdList,
    int32_t _transformBufferIndex,
    int32_t _materialBufferIndex,
    int32_t _textureBufferIndex,
    int32_t _offsetBufferIndex,
    BlendMode _blendMode,
    bool _isCulling) {

    if (modelGroups_.empty()) {
        return;
    }

    for (auto& [key, group] : modelGroups_) {
        if (group.instances.Empty()) {
            continue;
        }

        uint32_t submeshCount  = static_cast<uint32_t>(group.submeshOrder.size());

        for (uint32_t j = 0; j < submeshCount; ++j) {
            auto& batches = group.textureBatches[j];

            // このサブメッシュに一致するバッチがあるかチェック
            bool hasMatchingBatch = false;
            for (auto& batch : batches) {
                if (batch.blendMode == _blendMode && batch.isCulling == _isCulling) {
                    hasMatchingBatch = true;
                    break;
                }
            }
            if (!hasMatchingBatch) {
                continue;
            }

            TextureColorMesh* mesh = group.submeshOrder[j];

            // VB / IB セット
            _cmdList->IASetVertexBuffers(0, 1, &mesh->GetVBView());
            _cmdList->IASetIndexBuffer(&mesh->GetIBView());

            // Transform StructuredBuffer セット
            group.transformBuffers[j].SetForRootParameter(_cmdList, _transformBufferIndex);

            // Material StructuredBuffer セット
            group.materialBuffers[j].SetForRootParameter(_cmdList, _materialBufferIndex);

            // 一致するバッチのみ描画
            UINT indexCount = static_cast<UINT>(mesh->GetIndexSize());
            for (auto& batch : batches) {
                if (batch.blendMode != _blendMode || batch.isCulling != _isCulling) {
                    continue;
                }

                _cmdList->SetGraphicsRootDescriptorTable(_textureBufferIndex, batch.textureHandle);

                // SV_InstanceID は常に 0 から開始するため、オフセットをルート定数で渡す
                _cmdList->SetGraphicsRoot32BitConstant(_offsetBufferIndex, batch.startInstance, 0);

                _cmdList->DrawIndexedInstanced(
                    indexCount,
                    batch.instanceCount,
                    0, 0, 0);
            }
        }
    }
}

bool InstancedMeshManager::HasInstancesFor(BlendMode _blendMode, bool _isCulling) const {
    return hasInstanceFlags_[_isCulling ? 1 : 0][static_cast<size_t>(_blendMode)];
}

//==============================================================================
// InstancedMeshManager - Private Methods
//==============================================================================

void InstancedMeshManager::BuildSubmeshOrder(InstancedModelGroup& _group) {
    _group.submeshOrder.clear();
    _group.submeshLocalTransforms.clear();
    CollectSubmeshes(
        _group.modelData,
        &_group.modelData->rootNode,
        _group.submeshOrder,
        _group.submeshLocalTransforms);
}

void InstancedMeshManager::EnsureCapacity(InstancedModelGroup& _group, uint32_t _requiredCount) {
    if (_requiredCount == 0) {
        return;
    }

    auto device = Engine::GetInstance()->GetDxDevice()->device_;

    for (uint32_t j = 0; j < _group.transformBuffers.size(); ++j) {
        if (_requiredCount > _group.transformBuffers[j].Capacity()) {
            uint32_t newCapacity = _requiredCount * kGrowFactor;
            _group.transformBuffers[j].Resize(device, newCapacity);
            _group.materialBuffers[j].Resize(device, newCapacity);

            LOG_INFO(
                "InstancedMeshManager: Resized buffers for submesh [{}] to {} instances.",
                j, newCapacity);
        }
    }
}

ModelMeshData* InstancedMeshManager::GetOrCreatePrimitiveTemplate(
    const std::string& _typeName,
    std::function<void(TextureColorMesh*)> _meshCreator) {

    auto itr = primitiveTemplates_.find(_typeName);
    if (itr != primitiveTemplates_.end()) {
        return itr->second.get();
    }

    // 新規テンプレート作成
    auto meshData = std::make_unique<ModelMeshData>();

    // ルートノードにメッシュを1つ配置
    meshData->rootNode.name = _typeName;
    auto& mesh = meshData->meshGroup[_typeName];
    _meshCreator(&mesh);

    // ルートノードの Transform はデフォルト（単位）
    meshData->rootNode.transform = Transform();

    auto* ptr = meshData.get();
    primitiveTemplates_.emplace(_typeName, std::move(meshData));

    LOG_INFO("InstancedMeshManager: Created primitive template '{}'.", _typeName);
    return ptr;
}

void InstancedMeshManager::RebuildInstanceFlags() {
    for (auto& row : hasInstanceFlags_) {
        row.fill(false);
    }

    for (auto& [key, group] : modelGroups_) {
        for (auto& batches : group.textureBatches) {
            for (auto& batch : batches) {
                hasInstanceFlags_[batch.isCulling ? 1 : 0][static_cast<size_t>(batch.blendMode)] = true;
            }
        }
    }
}
