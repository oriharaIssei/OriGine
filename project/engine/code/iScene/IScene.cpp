#include "IScene.h"

/// stl
// container
#include <list>

/// engine
#define ENGINE_INCLUDE
#include "sceneManager/SceneManager.h"
#define RESOURCE_DIRECTORY
// Ecs
#define ENGINE_ECS

/// directX12Object
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
// lib
#include "lib/binaryIO/BinaryIO.h"

#include "engine/EngineInclude.h"

IScene::IScene(const std::string& sceneName) : name_(sceneName) {}

IScene::~IScene() {}

void IScene::Initialize() {

    // componentの登録
    registerComponents();

    // System の登録
    registerSystems();

    // 読み込み (component,System の登録のあと)
    SceneSerializer serializer;
    serializer.Deserialize(this->name_);

    ECSManager::getInstance()->SortPriorityOrderSystems();

#ifdef _DEBUG
    SceneManager* sceneManager = SceneManager::getInstance();
    if (sceneManager->inDebugMode()) {
        ECSManager::getInstance()->RunInitialize();
    }
#else
    ECSManager::getInstance()->RunInitialize();
#endif // _DEBUG
}

void IScene::registerComponents() {
    ECSManager* ecsManager = ECSManager::getInstance();

    ecsManager->registerComponent<Transform>();
    ecsManager->registerComponent<CameraTransform>();

    ecsManager->registerComponent<DirectionalLight>();
    ecsManager->registerComponent<PointLight>();
    ecsManager->registerComponent<SpotLight>();

    ecsManager->registerComponent<Rigidbody>();

    ecsManager->registerComponent<AABBCollider>();
    ecsManager->registerComponent<SphereCollider>();

    ecsManager->registerComponent<Emitter>();

    ecsManager->registerComponent<Audio>();

    ecsManager->registerComponent<ModelNodeAnimation>();
    ecsManager->registerComponent<PrimitiveNodeAnimation>();

    ecsManager->registerComponent<ModelMeshRenderer>();
    ecsManager->registerComponent<PlaneRenderer>();
    ecsManager->registerComponent<SpriteRenderer>();
    ecsManager->registerComponent<LineRenderer>();
}

void IScene::registerSystems() {
    ECSManager* ecsManager = ECSManager::getInstance();

    ecsManager->registerSystem<MoveSystemByRigidBody>();

    ecsManager->registerSystem<CollisionCheckSystem>();

    ecsManager->registerSystem<EmitterWorkSystem>();
    ecsManager->registerSystem<PrimitiveNodeAnimationWorkSystem>();

    ecsManager->registerSystem<ParticleRenderSystem>();
    ecsManager->registerSystem<SpriteRenderSystem>();
    ecsManager->registerSystem<TexturedMeshRenderSystem>();
    ecsManager->registerSystem<LineRenderSystem>();
    ecsManager->registerSystem<ColliderRenderingSystem>();

    /// postEffect は 全て任意
    // ecsManager->registerSystem<VignetteEffect>();
    // ecsManager->registerSystem<GrayscaleEffect>();
    // ecsManager->registerSystem<SmoothingEffect>();
}

void IScene::Finalize([[maybe_unused]] bool _isSave) {
#ifdef _DEBUG
    if (_isSave) {
        SceneSerializer serializer;
        serializer.Serialize(this->name_);
    }
#endif

    ECSManager* ecsManager = ECSManager::getInstance();
    ecsManager->FinalizeSystems();
    ecsManager->clearSystem();

    ecsManager->FinalizeComponentArrays();
    ecsManager->clearComponentArrays();

    ecsManager->clearAliveEntities();
}

// void IScene::LoadSceneEntity() {
//     ECSManager* ecsManager = ECSManager::getInstance();
//
//     // ===================================== 読み込み開始 ===================================== //
//     BinaryReader reader(kApplicationResourceDirectory + "/scene", name_ + ".scene");
//     if (!reader.ReadFile()) {
//         return;
//     }
//     // ------------------------------ エンティティ & component の読み込み ------------------------------//
//     {
//         GameEntity* loadedEntity = nullptr;
//         int32_t entitySize       = 0;
//         int32_t entityID         = 0;
//         reader.Read<int32_t>("entitySize", entitySize);
//         // エンティティの名前
//         std::string entityName;
//
//         // Entityが持つComponentの種類の数
//         int32_t hasComponentTypeSize                                           = 0;
//         const std::map<std::string, std::unique_ptr<IComponentArray>>& compMap = ecsManager->getComponentArrayMap();
//         // component の名前
//         std::string componentTypeName;
//
//         for (int32_t entityIndex = 0; entityIndex < entitySize; ++entityIndex) {
//             reader.ReadBeginGroup("Entity" + std::to_string(entityIndex));
//             reader.Read<std::string>("Name", entityName);
//
//             // 入力文字列を正規化
//             // 終端文字 の有り無しで 取得できない場合があったため,削除するように
//             entityName.erase(std::find(entityName.begin(), entityName.end(), '\0'), entityName.end());
//
//             entityID     = ecsManager->registerEntity(entityName);
//             loadedEntity = ecsManager->getEntity(entityID);
//
//             bool isUnique = false;
//             reader.Read<bool>("isUnique", isUnique);
//             if (isUnique) {
//                 ecsManager->registerUniqueEntity(entityName, loadedEntity);
//             }
//
//             { // System
//                 int32_t belongingSystemCount = 0;
//                 reader.Read<int32_t>("belongingSystemCount", belongingSystemCount);
//                 for (int32_t i = 0; i < belongingSystemCount; ++i) {
//                     std::string systemName;
//                     int32_t typeIndex = 0;
//                     reader.Read<int32_t>("System" + std::to_string(i) + "Type", typeIndex);
//                     reader.Read<std::string>("System" + std::to_string(i) + "Name", systemName);
//
//                     ISystem* system = ecsManager->getSystem(SystemType(typeIndex), systemName);
//
//                     if (!system) {
//                         continue;
//                     }
//                     system->addEntity(loadedEntity);
//                 }
//             }
//
//             { // Component
//                 reader.Read<int32_t>("hasComponentTypeSize", hasComponentTypeSize);
//
//                 for (int32_t componentIndex = 0; componentIndex < hasComponentTypeSize; ++componentIndex) {
//                     componentTypeName = "";
//                     reader.Read<std::string>("Component" + std::to_string(componentIndex) + "Name", componentTypeName);
//                     auto itr = compMap.find(componentTypeName);
//                     if (itr != compMap.end()) {
//                         itr->second->LoadComponent(loadedEntity, reader);
//                     }
//                 }
//             }
//             reader.ReadEndGroup();
//         }
//     }
//     // ------------------------------- System の 読み込み -------------------------------//
//     {
//         int32_t systemSizeByType = 0;
//         std::string systemName;
//
//         auto& systems           = ecsManager->getSystems();
//         int32_t systemTypeIndex = 0;
//         for (auto& systemsByType : systems) {
//             reader.Read<int32_t>(SystemTypeString[systemTypeIndex] + "Size", systemSizeByType);
//             for (int32_t systemIndex = 0; systemIndex < systemSizeByType; systemIndex++) {
//                 reader.ReadBeginGroup(SystemTypeString[systemTypeIndex] + std::to_string(systemIndex));
//                 reader.Read<std::string>("Name", systemName);
//
//                 auto itr = systemsByType.find(systemName);
//
//                 if (itr != systemsByType.end()) {
//                     // priority
//                     int32_t priority = 0;
//                     reader.Read<int32_t>("priority", priority);
//                     itr->second->setPriority(priority);
//
//                     bool isActive = false;
//                     reader.Read<bool>("isActive", isActive);
//                     itr->second->setIsActive(isActive);
//                 }
//                 reader.ReadEndGroup();
//             }
//             systemTypeIndex++;
//         }
//     }
//     // 読み込み 終了
//     reader.ReadEndGroup();
// }
//
// void IScene::SaveSceneEntity() {
//     ECSManager* ecsManager = ECSManager::getInstance();
//
//     // ===================================== 書き込み開始 ===================================== //
//     BinaryWriter writer(kApplicationResourceDirectory + "/scene", name_ + ".scene");
//     writer.WriteBegin();
//
//     // 収集済みの有効なEntity数を書き込む (Load側と対応)
//     std::list<GameEntity*> activeEntities;
//     {
//         int32_t entityID = 0;
//         for (auto& entity : ecsManager->getEntities()) {
//             if (!entity.isAlive()) {
//                 continue;
//             }
//             entityID = entity.getID();
//             activeEntities.push_front(ecsManager->getEntity(entityID));
//         }
//     }
//     writer.Write<int32_t>("entitySize", static_cast<int32_t>(activeEntities.size()));
//
//     // ------------------------------- エンティティ & コンポーネント の保存 -------------------------------//
//     auto& componentArrayMap = ecsManager->getComponentArrayMap();
//     std::vector<std::pair<std::string, IComponentArray*>> hasComponentTypeArray;
//
//     int32_t entityIndex = 0;
//     while (true) {
//         if (activeEntities.empty()) {
//             break;
//         }
//
//         writer.WriteBeginGroup("Entity" + std::to_string(entityIndex));
//
//         auto entity = activeEntities.front();
//         activeEntities.pop_front();
//
//         std::string entityName = entity->getDataType();
//         // 入力文字列を正規化
//         // 終端文字 の有り無しで 取得できない場合があったため,削除するように
//         entityName.erase(std::find(entityName.begin(), entityName.end(), '\0'), entityName.end());
//
//         writer.Write<std::string>("Name", entityName);
//         writer.Write<bool>("isUnique", entity->isUnique());
//
//         { // 所属するSystemを保存
//             auto& systems     = ecsManager->getSystems();
//             int32_t typeIndex = 0;
//             std::vector<std::pair<int32_t, std::string>> belongingSystems;
//             for (const auto& systemsByType : systems) {
//                 for (const auto& [systemName, system] : systemsByType) {
//                     if (system->hasEntity(entity)) {
//                         belongingSystems.push_back({typeIndex, systemName});
//                     }
//                 }
//                 typeIndex++;
//             }
//
//             writer.Write<int32_t>("belongingSystemCount", static_cast<int32_t>(belongingSystems.size()));
//             for (size_t i = 0; i < belongingSystems.size(); ++i) {
//                 writer.Write<int32_t>("System" + std::to_string(i) + "Type", belongingSystems[i].first);
//                 writer.Write<std::string>("System" + std::to_string(i) + "Name", belongingSystems[i].second);
//             }
//         }
//
//         { // Componentを保存
//             hasComponentTypeArray.clear();
//             for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
//                 if (!componentArray->hasEntity(entity)) {
//                     continue;
//                 }
//                 hasComponentTypeArray.push_back(std::make_pair(componentTypeName, componentArray.get()));
//             }
//             writer.Write<uint32_t>("hasComponentTypeSize", static_cast<uint32_t>(hasComponentTypeArray.size()));
//             int32_t componentIndex = 0;
//             for (const auto& [componentTypeName, componentArray] : hasComponentTypeArray) {
//                 writer.Write<std::string>("Component" + std::to_string(componentIndex) + "Name", componentTypeName);
//
//                 componentArray->SaveComponent(entity, writer);
//                 ++componentIndex;
//             }
//         }
//         writer.WriteEndGroup();
//         entityIndex++;
//     }
//
//     // ------------------------------- System の 保存 -------------------------------//
//     {
//         const auto& systems     = ecsManager->getSystems();
//         int32_t systemTypeIndex = 0;
//         for (const auto& systemsByType : systems) {
//             writer.Write<int32_t>(SystemTypeString[systemTypeIndex] + "Size", static_cast<int32_t>(systemsByType.size()));
//
//             int32_t systemIndex_ = 0;
//             for (const auto& [systemName, system] : systemsByType) {
//                 system->eraseDeadEntity();
//
//                 writer.WriteBeginGroup(SystemTypeString[systemTypeIndex] + std::to_string(systemIndex_++));
//                 writer.Write<std::string>("Name", systemName);
//                 writer.Write<int32_t>("priority", system->getPriority());
//                 writer.Write<bool>("isActive", system->isActive());
//             }
//             writer.WriteEndGroup();
//             systemTypeIndex++;
//         }
//     }
//
//     // ===================================== 書き込み終了 ===================================== //
//     writer.WriteEnd();
// }
