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
#include "directX12/Mesh.h"
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

    // to_json, from_json を friend として宣言
    friend void to_json(nlohmann::json& j, const Emitter& e);
    friend void from_json(const nlohmann::json& j, Emitter& e);

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

    void CalculateMaxSize();

    void PlayStart();
    void PlayStop();

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
    TextureMesh mesh_;
    IConstantBuffer<Material> material_;

    IStructuredBuffer<ParticleTransform> structuredTransform_;
    //=============== Texture ===============/
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

    Vec2f randMass_ = {1.f, 1.f};

    std::shared_ptr<ParticleKeyFrames> particleKeyFrames_ = nullptr;

#ifdef _DEBUG
    // 連番画像から uv Curveにするためのもの
    Vec2f tileSize_            = {};
    Vec2f textureSize_         = {};
    float tilePerTime_         = 0.f;
    float startAnimationTime_  = 0.f;
    float animationTimeLength_ = 0.f;
#endif // _DEBUG

    InterpolationType transformInterpolationType_ = InterpolationType::LINEAR;
    InterpolationType colorInterpolationType_     = InterpolationType::LINEAR;
    InterpolationType uvInterpolationType_        = InterpolationType::LINEAR;
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
    bool getIsActive() const { return isActive_; }
    // void setIsActive(bool _isActive) { isActive_ = _isActive; } // PlayStart を使え
    bool getIsLoop() const { return isLoop_; }
    void setIsLoop(bool _isLoop) { isLoop_ = _isLoop; }

    bool getIsBillBoard() const { return particleIsBillBoard_; }
    void setIsBillBoard(bool _isBillBoard) { particleIsBillBoard_ = _isBillBoard; }

    const Vec3f& getOriginePos() const { return originPos_; }
    void setOriginePos(const Vec3f& _pos) { originPos_ = _pos; }

    BlendMode getBlendMode() const { return blendMode_; }

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
        srvArray_ = DxSrvArrayManager::getInstance()->Create(256 + 64);
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
    void SaveComponent(GameEntity* _entity, nlohmann::json& _json) override {
        static_assert(HasToJson<Emitter>, "Emitter must have a to_json function");
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        nlohmann::json compVecJson = nlohmann::json::array();
        uint32_t index             = it->second;
        for (auto& comp : components_[index]) {
            compVecJson.emplace_back(comp);
        }
        _json[nameof<Emitter>()] = compVecJson;
    }

    /// @brief コンポーネントの読み込み
    void LoadComponent(GameEntity* _entity, nlohmann::json& _json) override {
        static_assert(HasFromJson<Emitter>, "Emitter must have a from_json function");
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            // エンティティが登録されていない場合は新規登録
            entityIndexBind_[_entity] = static_cast<uint32_t>(components_.size());
            components_.emplace_back();
            it = entityIndexBind_.find(_entity);
        }
        uint32_t index = it->second;
        // 現在のコンポーネントをクリア
        components_[index].clear();
        // JSON 配列からコンポーネントを読み込み
        for (const auto& compJson : _json) {
            components_[index].emplace_back(srvArray_.get());
            from_json(compJson, components_[index].back());
            components_[index].back().Initialize(_entity);
        }
    }

    /// @brief エンティティ登録（メモリ確保）
    void registerEntity(GameEntity* _entity, int32_t _entitySize = 1, bool _doInitialize = true) override {
        uint32_t index = static_cast<uint32_t>(components_.size());
        components_.push_back(std::vector<Emitter>(_entitySize, srvArray_.get()));
        if (_doInitialize) {
            for (auto& comp : components_.back()) {
                comp.Initialize(_entity);
            }
        }
        entityIndexBind_[_entity] = index;
    }

    /// @brief 値によるコンポーネントの追加
    void add(GameEntity* _hostEntity, const Emitter& _component, bool _doInit = true) {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            return;
        }
        uint32_t index = it->second;
        components_[index].emplace_back(srvArray_.get());
        components_[index].back() = _component;
        if (_doInit) {
            components_[index].back().Initialize(_hostEntity);
        }
    }

    /// @brief IComponent ポインタからコンポーネントの追加
    void addComponent(GameEntity* _hostEntity, IComponent* _component, bool _doInitialize) override {
        const Emitter* comp = dynamic_cast<const Emitter*>(_component);
        assert(comp != nullptr && "Invalid component type passed to addComponent");
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity, 1, _doInitialize);
            return;
        }
        uint32_t index = it->second;
        components_[index].emplace_back(std::move(*comp));
        if (_doInitialize) {
            components_[index].back().Initialize(_hostEntity);
        }
    }

    /// @brief デフォルト値によるコンポーネントの追加
    void addComponent(GameEntity* _hostEntity, bool _doInitialize) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity, 1, _doInitialize);
            return;
        }
        uint32_t index = it->second;
        components_[index].emplace_back(srvArray_.get());
        if (_doInitialize) {
            components_[index].back().Initialize(_hostEntity);
        }
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
        uint32_t index            = it->second;
        std::vector<Emitter>& vec = components_[index];

        vec[_componentIndex].Finalize();

        vec.erase(vec.begin() + _componentIndex);
    }

    void removeBackComponent(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        auto& vec      = components_[index];
        vec.back().Finalize();
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

// to_json が存在するかをチェックする concept

#pragma endregion
