#pragma once

/// stl
#include <memory>
#include <vector>

/// engine
// dx12object
#include "directX12/DxSrvArray.h"
#include "directX12/DxSrvArrayManager.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "directX12/ShaderManager.h"
// assets
#include "component/material/Material.h"
struct ParticleKeyFrames;
struct EmitterShape;
struct ParticleKeyFrames;
// object
#include "../Particle.h"
// component
#include "component/ComponentArray.h"
#include "component/transform/CameraTransform.h"
#include "component/transform/ParticleTransform.h"
#include "component/transform/Transform.h"

/// math
// shape
#include "EmitterShape.h"

class Emitter
    : public IComponent {

public:
    Emitter();
    Emitter(DxSrvArray* _srvArray);
    ~Emitter();

    void Initialize(GameEntity* _entity) override;
    //! ToDo :: Update,Draw の削除
    void Update(float _deltaTime);
    void UpdateParticle(float _deltaTime);
    void Draw(ID3D12GraphicsCommandList* _commandList);

    void Finalize() override;

    bool Edit() override;
    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

    void CalculateMaxSize();

private:
    void SpawnParticle();

#ifdef _DEBUG
    void EditEmitter();

    /// <summary>
    /// ShapeType に関する 編集項目
    /// </summary>
    void EditShapeType();

    /// <summary>
    /// Particle の 編集項目
    /// </summary>
    void EditParticle();
#endif // _DEBUG
private:
    DxSrvArray* srvArray_ = nullptr;

    Transform* parent_        = nullptr;
    Vec3f originPos_          = {0.f, 0.f, 0.f};
    uint32_t particleMaxSize_ = 34;

    std::vector<std::shared_ptr<Particle>> particles_;

    /// <summary>
    /// 頂点とMaterial を 併せ持つ
    /// </summary>
    std::shared_ptr<Model> particleModel_;
    IStructuredBuffer<ParticleTransform> structuredTransform_;
    //=============== Model & Texture ===============/
    std::string modelDirectory_  = "";
    std::string modelFileName_   = "";
    std::string textureFileName_ = "";
    int32_t textureIndex_        = 0;

    //=============== エミッター設定項目 ===============//
    BlendMode blendMode_ = BlendMode::None;
    bool isActive_       = false;
    bool isLoop_         = false;
    // emitter 生存時間
    float activeTime_     = 0.f;
    float leftActiveTime_ = 0.f;

    /// <summary>
    /// 一度に 生成される Particle の 数
    /// </summary>
    int32_t spawnParticleVal_ = 1;

    EmitterShapeType shapeType_ = EmitterShapeType::SPHERE;
    std::shared_ptr<EmitterShape> emitterSpawnShape_;

    float currentCoolTime_  = 0.f;
    float spawnCoolTime_    = 0.f;
    float particleLifeTime_ = 0.f;

    // billBoard 計算するかどうか
    bool particleIsBillBoard_ = true;

    //=============== パーティクル設定項目 ===============//
    Vec4f particleColor_   = {1.f, 1.f, 1.f, 1.f};
    Vec3f particleUvScale_ = {1.f, 1.f, 1.f};
    Vec3f particleUvRotate_;
    Vec3f particleUvTranslate_;

    int32_t updateSettings_ = 0;

    std::shared_ptr<ParticleKeyFrames> particleKeyFrames_ = nullptr;

    // ランダムな数値の範囲を設定するためのメンバ変数
    // ランダムではない場合 (min == max) になる
    Vec3f startParticleScaleMin_    = {1.f, 1.f, 1.f};
    Vec3f startParticleScaleMax_    = {1.f, 1.f, 1.f};
    Vec3f startParticleRotateMin_   = {0.f, 0.f, 0.f};
    Vec3f startParticleRotateMax_   = {0.f, 0.f, 0.f};
    Vec3f startParticleVelocityMin_ = {0.f, 0.f, 0.f};
    Vec3f startParticleVelocityMax_ = {0.f, 0.f, 0.f};

    Vec3f updateParticleScaleMin_    = {1.f, 1.f, 1.f};
    Vec3f updateParticleScaleMax_    = {1.f, 1.f, 1.f};
    Vec3f updateParticleRotateMin_   = {0.f, 0.f, 0.f};
    Vec3f updateParticleRotateMax_   = {0.f, 0.f, 0.f};
    Vec3f updateParticleVelocityMin_ = {0.f, 0.f, 0.f};
    Vec3f updateParticleVelocityMax_ = {0.f, 0.f, 0.f};

public:
    BlendMode getBlendMode() const { return blendMode_; }
    bool getIsActive() const { return isActive_; };

    void setParent(Transform* _parent) { parent_ = _parent; }
};

#pragma region
template <>
class ComponentArray<Emitter>
    : public IComponentArray {
public:
    using Emitter = Emitter;

    // コンストラクタ・デストラクタ
    ComponentArray()          = default;
    virtual ~ComponentArray() = default;

    // ─────────────────────────────
    //  基本関数
    // ─────────────────────────────
    /// @brief 内部データの初期化
    void Initialize(uint32_t _size) override {
        components_.clear();
        entityIndexBind_.clear();
        components_.reserve(_size);

        srvArray_ = DxSrvArrayManager::getInstance()->Create(64);
    }
    void Finalize() override {

        for (auto& compVec : components_) {
            for (auto& comp : compVec) {
                comp.Finalize();
            }
        }

        clear();
        srvArray_->Finalize();
        srvArray_.reset();
    }

    /// @brief コンポーネントの保存
    void SaveComponent(GameEntity* _entity, BinaryWriter& _writer) override;

    /// @brief コンポーネントの読み込み
    void LoadComponent(GameEntity* _entity, BinaryReader& _reader) override;

    /// @brief エンティティ登録（メモリ確保）
    void registerEntity(GameEntity* _entity, int32_t _entitySize = 1) override {
        uint32_t index = static_cast<uint32_t>(components_.size());
        components_.push_back(std::vector<Emitter>(_entitySize, srvArray_.get()));
        for (auto& comp : components_.back()) {
            comp.Initialize(_entity);
        }
        entityIndexBind_[_entity] = index;
    }

    /// @brief 値によるコンポーネントの追加
    void add(GameEntity* _hostEntity, const Emitter& _component) {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            return;
        }
        uint32_t index = it->second;
        components_[index].push_back(_component);
        components_[index].back().Initialize(_hostEntity);
    }

    /// @brief IComponent ポインタからコンポーネントの追加
    void addComponent(GameEntity* _hostEntity, IComponent* _component) override {
        const Emitter* comp = dynamic_cast<const Emitter*>(_component);
        assert(comp != nullptr && "Invalid component type passed to addComponent");
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            return;
        }
        uint32_t index = it->second;
        components_[index].push_back(std::move(*comp));
        components_[index].back().Initialize(_hostEntity);
    }

    /// @brief デフォルト値によるコンポーネントの追加
    void addComponent(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            return;
        }
        uint32_t index = it->second;
        components_[index].push_back(Emitter(srvArray_.get()));
        components_[index].back().Initialize(_hostEntity);
    }

    virtual void insertComponent(GameEntity* _hostEntity, IComponent* _component, int32_t _index) override {
        const Emitter* comp = dynamic_cast<const Emitter*>(_component);
        assert(comp != nullptr && "Invalid component type passed to addComponent");
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            return;
        }
        uint32_t index = it->second;
        components_[index].insert(components_[index].begin() + _index, std::move(*comp));
    }
    virtual void insertComponent(GameEntity* _hostEntity, int32_t _index) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            it = entityIndexBind_.find(_hostEntity);
            return;
        }
        uint32_t index = it->second;
        components_[index].insert(components_[index].begin() + _index, Emitter(srvArray_.get()));
    }

    void reserveEntity(GameEntity* _hostEntity, int32_t _size) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        components_[index].reserve(_size);
    }

    void resizeEntity(GameEntity* _hostEntity, int32_t _size) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        components_[index].reserve(_size);
    }

    /// @brief 指定インデックスのコンポーネント削除
    void removeComponent(GameEntity* _hostEntity, int32_t _componentIndex = 1) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        auto& vec      = components_[index];
        vec.erase(std::remove_if(vec.begin(), vec.end(), [vec, _componentIndex](Emitter& comp) {
            bool isRemove = &vec[_componentIndex] == &comp;
            if (isRemove) {
                comp.Finalize();
            }
            return isRemove;
        }),
            vec.end());
    }

    void removeBackComponent(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        auto& vec      = components_[index];
        vec.pop_back();
    }

    /// @brief 指定エンティティの全コンポーネント削除
    void clearComponent(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        for (auto& comp : components_[index]) {
            comp.Finalize();
        }
        components_[index].clear();
    }

    /// @brief 全コンポーネントのクリア
    void clear() override {
        for (auto& compArray : components_) {
            for (auto& comp : compArray) {
                comp.Finalize();
            }
        }
        components_.clear();
        entityIndexBind_.clear();
    }

    /// @brief エンティティ削除（インデックス解放）
    void deleteEntity(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        // 対象コンポーネントの後始末
        for (auto& comp : components_[index]) {
            comp.Finalize();
        }
        // components_から削除
        components_.erase(components_.begin() + index);
        // エントリ削除
        entityIndexBind_.erase(it);
        // index が削除された位置より大きいエンティティのインデックスを1減算
        for (auto& pair : entityIndexBind_) {
            if (pair.second > index) {
                pair.second--;
            }
        }
    }

    bool hasEntity(GameEntity* _hostEntity) override {
        return entityIndexBind_.find(_hostEntity) != entityIndexBind_.end();
    }

    int32_t entityCapacity(GameEntity* _hostEntity) const override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return 0;
        }
        uint32_t index = it->second;
        return static_cast<int32_t>(components_[index].size());
    }

    std::vector<std::vector<Emitter>>* getAllComponents() {
        return &components_;
    }

protected:
    // ─────────────────────────────
    //  メンバ変数
    // ─────────────────────────────
    std::vector<std::vector<Emitter>> components_;
    std::map<GameEntity*, uint32_t> entityIndexBind_;

    std::shared_ptr<DxSrvArray> srvArray_ = nullptr;

public:
    // ─────────────────────────────
    //  アクセッサー
    // ─────────────────────────────

    /// @brief エンティティのコンポーネント数取得
    int32_t getComponentSize(GameEntity* _entity) override {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return 0;
        }
        uint32_t index = it->second;
        return static_cast<int32_t>(components_[index].size());
    }
    /// @brief エンティティごとのコンポーネント配列取得
    std::vector<Emitter>* getComponents(GameEntity* _entity) {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].empty() ? nullptr : &components_[index];
    }

    /// @brief エンティティの 先頭のコンポーネント取得
    IComponent* getFrontComponent(GameEntity* _entity) override {
        return getComponent(_entity, 0);
    }
    /// @brief エンティティの 末尾のコンポーネント取得
    IComponent* getBackComponent(GameEntity* _entity) override {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].empty() ? nullptr : static_cast<IComponent*>(&components_[index].back());
    }

    /// @brief 動的コンポーネント取得
    Emitter* getDynamicComponent(GameEntity* _entity, uint32_t _index = 0) {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].size() <= _index ? nullptr : &components_[index][_index];
    }
    /// @brief 動的なコンポーネントの先頭を取得
    Emitter* getDynamicFrontComponent(GameEntity* _entity) {
        return getDynamicComponent(_entity, 0);
    }
    /// @brief 動的なコンポーネントの末尾を取得
    Emitter* getDynamicBackComponent(GameEntity* _entity) {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].empty() ? nullptr : &components_[index].back();
    }

    /// @brief IComponent 型としてコンポーネント取得
    IComponent* getComponent(GameEntity* _entity, uint32_t _index = 0) override {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].size() <= _index ? nullptr : &components_[index][_index];
    }
};

inline void ComponentArray<Emitter>::SaveComponent(GameEntity* _entity, BinaryWriter& _writer) {
    auto it = entityIndexBind_.find(_entity);
    if (it == entityIndexBind_.end()) {
        return;
    }

    std::string preGroupName      = _writer.getGroupName();
    std::string componentTypeName = "Emitter";
    _writer.WriteBeginGroup(preGroupName + componentTypeName);

    uint32_t index = it->second;
    _writer.Write<uint32_t>("size", static_cast<uint32_t>(components_[index].size()));

    int32_t compIndex = 0;
    for (auto& comp : components_[index]) {
        _writer.WriteBeginGroup(preGroupName + componentTypeName + std::to_string(compIndex++));
        comp.Save(_writer);
    }

    _writer.WriteBeginGroup(preGroupName);
}

inline void ComponentArray<Emitter>::LoadComponent(GameEntity* _entity, BinaryReader& _reader) {
    std::string preGroupName = _reader.getGroupName();

    uint32_t size;
    std::string componentTypeName = "Emitter";
    _reader.ReadBeginGroup(preGroupName + componentTypeName);

    _reader.Read<uint32_t>("size", size);
    registerEntity(_entity, size);
    auto& componentVec = components_[entityIndexBind_[const_cast<GameEntity*>(_entity)]];
    int32_t compIndex  = 0;
    for (auto& comp : componentVec) {
        _reader.ReadBeginGroup(preGroupName + componentTypeName + std::to_string(compIndex++));
        comp.Load(_reader);
    }
    _reader.ReadBeginGroup(preGroupName);
}

#pragma endregion
