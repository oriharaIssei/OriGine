#pragma once

/// stl
#include <assert.h>
#include <memory>
// container
#include <map>
#include <vector>
// utility
#include <concepts>
#include <stdint.h>

/// engine
// ECS
#include "component/IComponent.h"
#include "Entity.h"

///====================================================================================
// IComponentArray Interface
///====================================================================================
/// <summary>
/// ECS の各コンポーネント配列を統一的に管理するためのインターフェース。
/// </summary>
class IComponentArray {
public:
    IComponentArray()          = default;
    virtual ~IComponentArray() = default;

    /// @brief 指定サイズで初期化する
    virtual void Initialize(uint32_t _size) = 0;
    virtual void Finalize()                 = 0;

    /// @brief 指定エンティティのコンポーネントを BinaryWriter で保存する
    virtual void SaveComponent(GameEntity* _entity, BinaryWriter& _writer) = 0;

    /// @brief 指定エンティティのコンポーネントを BinaryReader で読み込む
    virtual void LoadComponent(GameEntity* _entity, BinaryReader& _reader) = 0;

    virtual void reserveEntity(GameEntity* _hostEntity, int32_t _entitySize) = 0;

    virtual void resizeEntity(GameEntity* _hostEntity, int32_t _entitySize) = 0;

    /// @brief 登録されている全コンポーネント配列をクリアする
    virtual void clear() = 0;

    /// @brief 指定エンティティの全コンポーネントをクリアする
    virtual void clearComponent(GameEntity* _hostEntity) = 0;

    /// @brief 指定エンティティのコンポーネント数を取得する
    virtual int32_t getComponentSize(GameEntity* _entity) = 0;

    /// @brief 指定エンティティのコンポーネントを取得する
    virtual IComponent* getComponent(GameEntity* _entity, uint32_t _index = 0) = 0;

    virtual IComponent* getFrontComponent(GameEntity* _entity) {
        return getComponent(_entity, 0);
    }
    virtual IComponent* getBackComponent(GameEntity* _entity) = 0;

    /// @brief エンティティを登録し、初期のコンポーネント領域を確保する
    virtual void registerEntity(GameEntity* _entity, int32_t _entitySize = 1, bool _doInitialize = true) = 0;

    /// @brief IComponent を用いてコンポーネントを追加する
    virtual void addComponent(GameEntity* _hostEntity, IComponent* _component, bool _doInitialize = true) = 0;

    /// @brief デフォルト値によるコンポーネントを追加する
    virtual void addComponent(GameEntity* _hostEntity, bool _doInitialize = true) = 0;

    virtual void insertComponent(GameEntity* _hostEntity, IComponent* _component, int32_t _index) = 0;
    virtual void insertComponent(GameEntity* _hostEntity, int32_t _index)                         = 0;

    /// @brief 指定エンティティの特定インデックスのコンポーネントを削除する
    virtual void removeComponent(GameEntity* _hostEntity, int32_t _componentIndex = 1) = 0;

    /// @brief 末尾のエンティティを削除
    virtual void removeBackComponent(GameEntity* _hostEntity) = 0;

    /// @brief エンティティの全コンポーネントを削除し、インデックスを解放する
    virtual void deleteEntity(GameEntity* _hostEntity) = 0;

    virtual bool hasEntity(GameEntity* _hostEntity) = 0;

    /// @brief エンティティのコンポーネント数を取得する
    virtual int32_t entityCapacity(GameEntity* _hostEntity) const = 0;
};

///====================================================================================
// ComponentArray
///====================================================================================
template <IsComponent componentType>
class ComponentArray
    : public IComponentArray {
public:
    using ComponentType = componentType;

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
    }
    void Finalize() {
        clear();
    }

    /// @brief コンポーネントの保存
    void SaveComponent(GameEntity* _entity, BinaryWriter& _writer) override;

    /// @brief コンポーネントの読み込み
    void LoadComponent(GameEntity* _entity, BinaryReader& _reader) override;

    /// @brief エンティティ登録（メモリ確保）
    void registerEntity(GameEntity* _entity, int32_t _entitySize = 1, bool _doInitialize = true) override {
        uint32_t index = static_cast<uint32_t>(components_.size());
        components_.push_back(std::vector<componentType>());
        components_.back().resize(_entitySize);

        if (_doInitialize) {
            for (auto& comp : components_.back()) {
                comp.Initialize(_entity);
            }
        }

        entityIndexBind_[_entity] = index;
    }

    /// @brief 値によるコンポーネントの追加
    void add(GameEntity* _hostEntity, const componentType& _component, bool _doInitialize = true) {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity, 1, _doInitialize);
            return;
        }
        uint32_t index = it->second;
        components_[index].push_back(_component);
        if (_doInitialize) {
            components_[index].back().Initialize(_hostEntity);
        }
    }

    /// @brief IComponent ポインタからコンポーネントの追加
    void addComponent(GameEntity* _hostEntity, IComponent* _component, bool _doInitialize = true) override {
        const componentType* comp = dynamic_cast<const componentType*>(_component);
        assert(comp != nullptr && "Invalid component type passed to addComponent");
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity, 1, _doInitialize);
            return;
        }
        uint32_t index = it->second;
        components_[index].push_back(std::move(*comp));
        if (_doInitialize) {
            components_[index].back().Initialize(_hostEntity);
        }
    }

    /// @brief デフォルト値によるコンポーネントの追加
    void addComponent(GameEntity* _hostEntity, bool _doInitialize = true) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity, 1, _doInitialize);
            return;
        }
        uint32_t index = it->second;
        components_[index].push_back(ComponentType());
        if (_doInitialize) {
            components_[index].back().Initialize(_hostEntity);
        }
    }

    virtual void insertComponent(GameEntity* _hostEntity, IComponent* _component, int32_t _index) override {
        const componentType* comp = dynamic_cast<const componentType*>(_component);
        assert(comp != nullptr && "Invalid component type passed to addComponent");
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        if (_index > components_[index].size()) {
            return;
        }
        components_[index].insert(components_[index].begin() + _index, std::move(*comp));
    }
    virtual void insertComponent(GameEntity* _hostEntity, int32_t _index) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        if (_index > components_[index].size()) {
            return;
        }
        components_[index].insert(components_[index].begin() + _index, ComponentType());
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
        uint32_t index                  = it->second;
        std::vector<ComponentType>& vec = components_[index];

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
        return static_cast<int32_t>(components_[index].capacity());
    }

    std::vector<std::vector<componentType>>* getAllComponents() {
        return &components_;
    }

protected:
    // ─────────────────────────────
    //  メンバ変数
    // ─────────────────────────────
    std::vector<std::vector<componentType>> components_;
    std::map<GameEntity*, uint32_t> entityIndexBind_;

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
    std::vector<componentType>* getComponents(GameEntity* _entity) {
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
    componentType* getDynamicComponent(GameEntity* _entity, uint32_t _index = 0) {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].size() <= _index ? nullptr : &components_[index][_index];
    }
    /// @brief 動的なコンポーネントの先頭を取得
    componentType* getDynamicFrontComponent(GameEntity* _entity) {
        return getDynamicComponent(_entity, 0);
    }
    /// @brief 動的なコンポーネントの末尾を取得
    componentType* getDynamicBackComponent(GameEntity* _entity) {
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

template <IsComponent componentType>
inline void ComponentArray<componentType>::SaveComponent(GameEntity* _entity, BinaryWriter& _writer) {
    auto it = entityIndexBind_.find(_entity);
    if (it == entityIndexBind_.end()) {
        return;
    }

    std::string preGroupName      = _writer.getGroupName();
    std::string componentTypeName = nameof<componentType>();
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

template <IsComponent componentType>
inline void ComponentArray<componentType>::LoadComponent(GameEntity* _entity, BinaryReader& _reader) {
    std::string preGroupName = _reader.getGroupName();

    uint32_t size;
    std::string componentTypeName = nameof<componentType>();
    _reader.ReadBeginGroup(preGroupName + componentTypeName);

    _reader.Read<uint32_t>("size", size);
    registerEntity(_entity, size);
    auto& componentVec = components_[entityIndexBind_[const_cast<GameEntity*>(_entity)]];

    int32_t compIndex = 0;
    for (auto& comp : componentVec) {
        _reader.ReadBeginGroup(preGroupName + componentTypeName + std::to_string(compIndex++));
        comp.Load(_reader);
    }

    _reader.ReadBeginGroup(preGroupName);
}
