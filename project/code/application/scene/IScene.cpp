#include "IScene.h"

/// engine
#include "Engine.h"
#include "manager/SceneManager.h"
// Ecs
#include "ECSManager.h"
// component
#include "component/collider/Collider.h"
#include "component/IComponent.h"
#include "component/material/Material.h"
#include "component/renderer/MeshRender.h"
#include "component/renderer/Sprite.h"
#include "component/transform/CameraTransform.h"
// #include "component/transform/ParticleTransform.h"
#include "component/collider/Collider.h"
#include "component/transform/Transform.h"
// system
#include "system/collision/CollisionCheckSystem.h"
#include "system/render/SpritRenderSystem.h"
#include "system/render/TexturedMeshRenderSystem.h"

/// directX12Object
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
// lib
#include "lib/binaryIO/BinaryIO.h"

IScene::IScene(const std::string& sceneName) : name_(sceneName) {
}

IScene::~IScene() {}

void IScene::Init() {
    ECSManager* ecsManager = ECSManager::getInstance();
    ecsManager->Init();
    ecsManager->ComponentArraysInit();
// componentの登録
#pragma region "RegisterComponent"
    ecsManager->registerComponent<Transform>();
    ecsManager->registerComponent<CameraTransform>();
    ecsManager->registerComponent<ModelMeshRenderer>();
    ecsManager->registerComponent<SpriteRenderer>();
    ecsManager->registerComponent<AABBCollider>();
    ecsManager->registerComponent<SphereCollider>();
#pragma endregion "RegisterComponent"

// System の登録
#pragma region "RegisterSystem"
    ecsManager->registerSystem<TexturedMeshRenderSystem>();
    ecsManager->registerSystem<SpritRenderSystem>();
    ecsManager->registerSystem<CollisionCheckSystem>();
#pragma endregion "RegisterSystem"
    ECSManager::getInstance()->SortPriorityOrderSystems();

    // 読み込み (component,System の登録のあと)
    LoadSceneEntity();
}

void IScene::Finalize() {
#ifdef _DEBUG
    SaveSceneEntity();
#endif
    sceneRtvArray_.reset();
    sceneSrvArray_.reset();

    ECSManager* ecsManager = ECSManager::getInstance();
    ecsManager->clearComponents();
    ecsManager->clearSystem();
    ecsManager->clearEntity();
}

void IScene::LoadSceneEntity() {
    ECSManager* ecsManager = ECSManager::getInstance();

    // ===================================== 読み込み開始 ===================================== //
    BinaryReader reader("resource/scene", name_ + ".scene");
    reader.ReadBegin();

    // ------------------------------ エンティティ & component の読み込み ------------------------------//
    {
        GameEntity* loadedEntity = nullptr;
        int32_t entitySize       = 0;
        int32_t entityID         = 0;
        reader.Read<int32_t>(entitySize);
        // エンティティの名前
        std::string entityName;

        // Entityが持つComponentの種類の数
        int32_t hasComponentTypeSize                                           = 0;
        const std::map<std::string, std::unique_ptr<IComponentArray>>& compMap = ecsManager->getComponentArrayMap();
        // component の名前
        std::string componentTypeName;

        for (int32_t entityIndex = 0; entityIndex < entitySize; ++entityIndex) {
            reader.Read<std::string>(entityName);

            entityID     = ecsManager->registerEntity(entityName);
            loadedEntity = ecsManager->getEntity(entityID);

            reader.Read<int32_t>(hasComponentTypeSize);

            for (int32_t componentIndex = 0; componentIndex < hasComponentTypeSize; ++componentIndex) {
                reader.Read<std::string>(componentTypeName);
                auto itr = compMap.find(componentTypeName);
                if (itr != compMap.end()) {
                    itr->second->LoadComponent(loadedEntity, reader);
                }
            }
        }
    }

    // ------------------------------- System の 読み込み -------------------------------//
    {
        int32_t systemSizeByType    = 0;
        int32_t systemHasEntitySize = 0;
        std::string systemName;
        int32_t entityID = 0;

        auto& systems = ecsManager->getSystems();
        for (auto& systemsByType : systems) {
            reader.Read<int32_t>(systemSizeByType);
            for (int32_t i = 0; i < systemSizeByType; i++) {
                reader.Read<std::string>(systemName);
                auto itr = systemsByType.find(systemName);

                if (itr != systemsByType.end()) {
                    // priority
                    int32_t priority = 0;
                    reader.Read<int32_t>(priority);
                    itr->second->setPriority(priority);

                    // entities
                    reader.Read<int32_t>(systemHasEntitySize);
                    for (int32_t j = 0; j < systemHasEntitySize; j++) {
                        reader.Read<int32_t>(entityID);
                        itr->second->addEntity(ecsManager->getEntity(entityID));
                    }
                }
            }
        }
    }
    // 読み込み 終了
    reader.ReadEnd();
}

void IScene::SaveSceneEntity() {
    ECSManager* ecsManager = ECSManager::getInstance();

    // ===================================== 書き込み開始 ===================================== //
    BinaryWriter writer("resource/scene", name_ + ".scene");
    writer.WriteBegin();

    // 収集済みの有効なEntity数を書き込む (Load側と対応)
    std::vector<GameEntity*> activeEntities;
    {
        int32_t entityID = 0;
        for (auto& entity : ecsManager->getEntities()) {
            entityID = entity.getID();
            if (entityID < 0) {
                continue;
            }
            activeEntities.push_back(ecsManager->getEntity(entityID));
        }
    }
    writer.Write<int32_t>(static_cast<int32_t>(activeEntities.size()));

    // ------------------------------- エンティティ & コンポーネント の保存 -------------------------------//
    auto& componentArrayMap = ecsManager->getComponentArrayMap();
    std::vector<std::pair<std::string, IComponentArray*>> hasComponentTypeArray;

    for (auto entity : activeEntities) {
        writer.Write<std::string>(entity->getDataType());

        hasComponentTypeArray.clear();
        for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
            if (componentArray->getComponent(entity) == nullptr) {
                continue;
            }
            hasComponentTypeArray.push_back(std::make_pair(componentTypeName, componentArray.get()));
        }
        writer.Write<uint32_t>(static_cast<uint32_t>(hasComponentTypeArray.size()));
        for (const auto& [componentTypeName, componentArray] : hasComponentTypeArray) {
            writer.Write<std::string>(componentTypeName);
            componentArray->SaveComponent(entity, writer);
        }
    }

    // ------------------------------- System の 保存 -------------------------------//
    {
        const auto& systems = ecsManager->getSystems();
        for (const auto& systemsByType : systems) {
            writer.Write<int32_t>(static_cast<int32_t>(systemsByType.size()));
            for (const auto& [systemName, system] : systemsByType) {
                writer.Write<std::string>(systemName);
                writer.Write<int32_t>(system->getPriority());
                writer.Write<int32_t>(static_cast<int32_t>(system->getEntities().size()));
                for (const auto& entity : system->getEntities()) {
                    writer.Write<int32_t>(entity->getID());
                }
            }
        }
    }

    // ===================================== 書き込み終了 ===================================== //
    writer.WriteEnd();
}
