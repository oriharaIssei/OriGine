#include "ComponentRepository.h"

/// ECS
// component
#include "component/ComponentRegistry.h"

/// profiler
#include "profiler/CallCounter.h"

/// logger
#include "logger/Logger.h"

using namespace OriGine;

ComponentRepository::ComponentRepository(){
	// 添字を型IDとして使うため、最初から上限ぶんの枠を確保しておく。
	// 使わない型は nullptr の穴のまま残す。詰め直さないので添字が動かない。
	componentArrays_.resize(kMaxComponentTypes);
}
ComponentRepository::~ComponentRepository() = default;

/// <summary>
/// 全てのコンポーネント配列をクリアする.
/// </summary>
void ComponentRepository::Clear(){
	for(auto& componentArray : componentArrays_){
		if(!componentArray){
			continue;
		}
		componentArray->Finalize();
		componentArray.reset();
	}
	// 枠(size)は kMaxComponentTypes のまま保つ。型IDは ComponentRegistry がプロセス全体で
	// 持つものなので、ここでは何も無効化しない。GetComponentArray<T>() のキャッシュを
	// 戻す必要が無いのはこのため(シーンを作り直しても型IDの意味は変わらない)。
}

/// <summary>
/// 指定した型名のコンポーネント配列を登録する
/// </summary>
bool ComponentRepository::RegisterComponentArray(const std::string& _compTypeName){
	const uint32_t typeId = ComponentRegistry::GetInstance()->AcquireTypeId(_compTypeName);
	if(typeId >= kMaxComponentTypes){
		LOG_ERROR("ComponentRepository: type id is not available for type: {}",_compTypeName);
		return false;
	}

	if(componentArrays_[typeId]){
		LOG_WARN("ComponentRepository: ComponentArray already registered for type: {}",_compTypeName);
		return false;
	}

	if(!ComponentRegistry::GetInstance()->HasComponentArray(_compTypeName)){
		LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}",_compTypeName);
		return false;
	}

	// ComponentRegistryに登録済みのファクトリからComponentArrayの実体を複製生成する
	componentArrays_[typeId] = ComponentRegistry::GetInstance()->CloneComponentArray(_compTypeName);
	componentArrays_[typeId]->Initialize(1000);
	return true;
}

/// <summary>
/// 指定した型名のコンポーネント配列を登録解除する
/// </summary>
void ComponentRepository::UnregisterComponentArray(const std::string& _typeName,bool _isFinalize){
	const uint32_t typeId = ComponentRegistry::GetInstance()->FindTypeId(_typeName);
	if(typeId >= kMaxComponentTypes || !componentArrays_[typeId]){
		return;
	}

	if(_isFinalize){
		componentArrays_[typeId]->Finalize();
	}

	// 詰めずに穴を空けるだけにする。添字は「位置」ではなく型IDなので、
	// ここで要素を消しても他の型の添字は一切動かない。
	componentArrays_[typeId].reset();
}

/// <summary>
/// 指定した型名のコンポーネント配列を取得する
/// </summary>
IComponentArray* ComponentRepository::GetComponentArray(const std::string& _typeName){
	PROFILE_COUNT("ComponentRepository::GetComponentArray(string)");
	const uint32_t typeId = ComponentRegistry::GetInstance()->AcquireTypeId(_typeName);
	if(typeId >= kMaxComponentTypes){
		LOG_ERROR("ComponentRepository: type id is not available for type: {}",_typeName);
		return nullptr;
	}

	if(!componentArrays_[typeId]){
		// 未登録の場合はここで遅延登録する(テンプレート版と挙動を揃えること)
		if(!RegisterComponentArray(_typeName)){
			LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}",_typeName);
			return nullptr;
		}
	}
	return componentArrays_[typeId].get();
}

/// <summary>
/// 指定したエンティティにコンポーネントを追加する
/// </summary>
void ComponentRepository::AddComponent(Scene* _scene,const std::string& _compTypeName,const EntityHandle& _handle){
	auto* componentArray = GetComponentArray(_compTypeName);
	if(componentArray){
		componentArray->AddComponent(_scene,_handle);
	} else{
		LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}",_compTypeName);
	}
}

/// <summary>
/// 指定したエンティティにコンポーネント群を追加する
/// </summary>
void ComponentRepository::AddComponent(Scene* _scene,const std::vector<std::string>& _compTypeNames,const EntityHandle& _handle){
	for(const auto& compTypeName : _compTypeNames){
		AddComponent(_scene,compTypeName,_handle);
	}
}

/// <summary>
/// 指定したエンティティからコンポーネントを削除する
/// </summary>
void ComponentRepository::RemoveComponent(const std::string& _compTypeName,const EntityHandle& _handle,int32_t _compIndex){
	auto componentArray = GetComponentArray(_compTypeName);
	if(componentArray){
		componentArray->RemoveComponent(_handle,_compIndex);
	} else{
		LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}",_compTypeName);
	}
}

/// <summary>
/// 指定したエンティティから全てのコンポーネントを削除する。
/// 型ごとの ComponentArray を1つずつ回り、各配列に「このEntityの分だけ消して」と依頼する形を取る。
/// 呼び出し側(Scene::ExecuteDeleteEntities)は、このコンポーネント一括削除 -> システムからの登録解除 ->
/// EntityRepositoryでのEntity実体削除、という順序を守って呼び出す。逆順にすると、
/// システムやコンポーネントが解放済み/無効化済みのエンティティを指したままになってしまう
/// </summary>
void ComponentRepository::RemoveEntity(const EntityHandle& _handle){
	for(const auto& componentArray : componentArrays_){
		if(!componentArray){
			// 未登録の型は穴として残っているので飛ばす
			continue;
		}
		componentArray->RemoveAllComponents(_handle);
	}
}

/// <summary>
/// 指定したエンティティが持つ全てのコンポーネントを取得する
/// </summary>
std::unordered_map<std::string,std::vector<IComponent*>> OriGine::ComponentRepository::GetAllComponentsOfEntity(const EntityHandle& _handle){
	std::unordered_map<std::string,std::vector<IComponent*>> result;

	// 型名は ComponentRegistry からの逆引きで得る(この経路はエディタ/シリアライズ用で低頻度)
	ComponentRegistry* registry = ComponentRegistry::GetInstance();
	for(uint32_t typeId = 0; typeId < static_cast<uint32_t>(componentArrays_.size()); ++typeId){
		const auto& componentArray = componentArrays_[typeId];
		if(!componentArray || !componentArray->HasEntity(_handle)){
			continue;
		}
		auto comps = componentArray->GetIComponents(_handle);
		if(comps.empty()){
			// 実体を持たない型は結果に含めない
			continue;
		}
		result[registry->GetTypeName(typeId)] = comps;
	}

	return result;
}


uint32_t ComponentRepository::GetComponentCount() const{
	// componentArrays_ は常に kMaxComponentTypes 個ぶん確保されているので、
	// size() ではなく実体のある枠の数を数える
	uint32_t count = 0;
	for(const auto& componentArray : componentArrays_){
		if(componentArray){
			++count;
		}
	}
	return count;
}

const std::vector<std::unique_ptr<IComponentArray>>& ComponentRepository::GetComponentArrayMap() const{
	return componentArrays_;
}

std::vector<std::unique_ptr<IComponentArray>>& ComponentRepository::GetComponentArrayMapRef(){
	return componentArrays_;
}
