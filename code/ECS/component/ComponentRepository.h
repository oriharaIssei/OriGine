#pragma once

#include "ComponentArray.h"
#include "ComponentRegistry.h"

/// profiler
#include "profiler/CallCounter.h"

/// stl
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include <cstdint>

namespace OriGine {

	/// <summary>
	/// Component Repository
	/// ComponentRepositoryは実際にシーンで使用されるコンポーネントの実体を保持する.
	/// </summary>
	class ComponentRepository final{
	public:
		ComponentRepository();
		~ComponentRepository();

		/// <summary>
		/// 全てのコンポーネント配列をクリアする.
		/// </summary>
		void Clear();

		/// <summary>
		/// 指定した型のコンポーネント配列を登録する
		/// </summary>
		/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
		/// <returns>登録ができた ＝ true ,できなかった = false</returns>
		template <IsComponent ComponentType>
		bool RegisterComponentArray();

		/// <summary>
		/// 指定した型名のコンポーネント配列を登録する
		/// </summary>
		/// <param name="_compTypeName">コンポーネントの型名</param>
		/// <returns>登録ができた ＝ true ,できなかった = false</returns>
		bool RegisterComponentArray(const std::string& _compTypeName);
		/// <summary>
		/// 指定した型名のコンポーネント配列を登録解除する
		/// </summary>
		/// <param name="_typeName">コンポーネントの型名</param>
		/// <param name="_isFinalize">Finalizeを呼び出すかどうか</param>
		void UnregisterComponentArray(const std::string& _typeName,bool _isFinalize = true);

		/// <summary>
		/// 指定した型のコンポーネント配列を取得する
		/// </summary>
		/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
		/// <returns>コンポーネント配列</returns>
		template <IsComponent ComponentType>
		ComponentArray<ComponentType>* GetComponentArray();
		/// <summary>
		/// 指定した型名のコンポーネント配列を取得する
		/// </summary>
		/// <param name="_typeName">コンポーネントの型名</param>
		IComponentArray* GetComponentArray(const std::string& _typeName);

		/// <summary>
		/// 指定したエンティティが持つ指定した型のコンポーネント群を取得する
		/// </summary>
		/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
		/// <param name="_handle">コンポーネントを持つエンティティ</param>
		/// <returns> _handleが持つコンポーネント郡 </returns>
		template <IsComponent ComponentType>
		std::vector<ComponentType>& GetComponents(const EntityHandle& _handle);

		/// <summary>
		/// 指定したエンティティが持つ指定した型のコンポーネントを取得する
		/// </summary>
		/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
		/// <param name="_handle">コンポーネントを持つエンティティ</param>
		/// <param name="_index">コンポーネントのインデックス</param>
		/// <returns> _handleが持つコンポーネント </returns>
		template <IsComponent ComponentType>
		ComponentType* GetComponent(const EntityHandle& _handle,uint32_t _index = 0);

		/// <summary>
		/// 指定したHandleのコンポーネントを取得する
		/// </summary>
		/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
		/// <param name="_handle">コンポーネントを持つエンティティ</param>
		/// <param name="_index">コンポーネントのインデックス</param>
		/// <returns> _handleが持つコンポーネント </returns>
		template <IsComponent ComponentType>
		ComponentType* GetComponent(ComponentHandle _handle);

		/// <summary>
		/// 指定したエンティティにコンポーネントを追加する
		/// </summary>
		/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
		/// <param name="_handle">コンポーネントを追加するエンティティ</param>
		/// <param name="_doInitialize">追加したコンポーネントのInitializeを呼び出すかどうか</param>
		template <IsComponent... ComponentType>
		void AddComponent(Scene* _scene,const EntityHandle& _handle);
		/// <summary>
		/// 指定したエンティティにコンポーネントを追加する
		/// </summary>
		/// <param name="_compTypeName">コンポーネントの型名</param>
		/// <param name="_handle">コンポーネントを追加するエンティティ</param>
		/// <param name="_doInitialize">追加したコンポーネントのInitializeを呼び出すかどうか</param>
		void AddComponent(Scene* _scene,const std::string& _compTypeName,const EntityHandle& _handle);
		/// <summary>
		/// 指定したエンティティにコンポーネント群を追加する
		/// </summary>
		/// <param name="_compTypeNames">コンポーネントの型名群</param>
		/// <param name="_handle">コンポーネントを追加するエンティティ</param>
		/// <param name="_doInitialize">追加したコンポーネントのInitializeを呼び出すかどうか</param>
		void AddComponent(Scene* _scene,const std::vector<std::string>& _compTypeNames,const EntityHandle& _handle);

		/// <summary>
		/// 指定したエンティティからコンポーネントを削除する
		/// </summary>
		/// <param name="_compTypeName">削除するコンポーネントの型名</param>
		/// <param name="_handle">コンポーネントを削除されるエンティティ</param>
		/// <param name="_compIndex">削除するコンポーネントのインデックス</param>
		void RemoveComponent(const std::string& _compTypeName,const EntityHandle& _handle,int32_t _compIndex = 0);

		/// <summary>
		/// 指定したエンティティからコンポーネント群を削除
		/// </summary>
		/// <param name="_compTypeNames">削除するコンポーネントの型名群</param>
		/// <param name="_handle">コンポーネントを削除されるエンティティ</param>
		/// <param name="_doFinalize">終了処理を呼び出すかどうか</param>
		template <IsComponent ComponentType>
		void RemoveComponent(const EntityHandle& _handle);

		/// <summary>
		/// 指定したエンティティから全てのコンポーネントを削除する
		/// </summary>
		/// <param name="_handle">コンポーネントを削除されるエンティティ</param>
		void RemoveEntity(const EntityHandle& _handle);

	private:
		/// <summary>
		/// コンポーネント配列の実体。**添字は ComponentRegistry が採番した型ID**であって、
		/// 「この配列の何番目に入れたか」ではない。
		///
		/// この区別が本質。添字が位置だった頃は、要素を消すと以降の添字がずれ、
		/// シーンを作り直すと同じ番号が別の型を指した。型IDはプロセス全体で不変なので、
		/// 穴が空いても他の型に影響せず、シーンを跨いでも意味が変わらない。
		///
		/// 生成時に kMaxComponentTypes 個ぶん確保し、未登録の型は nullptr の穴として残す
		/// (64 * sizeof(unique_ptr) = 512B / シーン。詰め直さないための対価としては安い)。
		/// </summary>
		std::vector<std::unique_ptr<IComponentArray>> componentArrays_;

	public:
		uint32_t GetComponentCount() const;

		const std::vector<std::unique_ptr<IComponentArray>>& GetComponentArrayMap() const;
		std::vector<std::unique_ptr<IComponentArray>>& GetComponentArrayMapRef();

	};

	template <IsComponent ComponentType>
	inline bool ComponentRepository::RegisterComponentArray(){
		const uint32_t typeId = GetComponentTypeId<ComponentType>();
		if(typeId >= kMaxComponentTypes){
			// kInvalidComponentTypeId(上限超過)もこの比較で弾ける
			LOG_ERROR("ComponentRepository: type id is not available for type: {}",nameof<ComponentType>());
			return false;
		}

		if(componentArrays_[typeId]){
			LOG_WARN("ComponentRepository: ComponentArray already registered for type: {}",nameof<ComponentType>());
			return false;
		}

		if(!ComponentRegistry::GetInstance()->HasComponentArray<ComponentType>()){
			LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}",nameof<ComponentType>());
			return false;
		}

		// ComponentRegistryに登録済みのファクトリからComponentArrayの実体を複製生成する
		componentArrays_[typeId] = ComponentRegistry::GetInstance()->CloneComponentArray<ComponentType>();
		componentArrays_[typeId]->Initialize(1000);
		return true;
	}

	template <IsComponent ComponentType>
	inline ComponentArray<ComponentType>* ComponentRepository::GetComponentArray(){
#if ORIGINE_CALL_COUNTER_ENABLED
		// この関数は1フレームに20万回超呼ばれるため、カウンタ名の組み立て(nameof<ComponentType>() を含み、
		// それ自体がヒープ確保を伴う)をホットパスで毎回行うわけにはいかない。関数ローカル static は
		// ComponentType ごとに1度しか初期化されないので、名前の解決はそこで済ませ、以降はハンドル経由の
		// 配列添字だけが残る。ブロックごとガードしているのは、計測器を切ったビルドに「一度きりとはいえ
		// 計測器なしのコードには存在しなかった確保」を紛れ込ませないため。
		static const std::string s_callCounterName = "ComponentRepository::GetComponentArray<" + nameof<ComponentType>() + ">";
		PROFILE_COUNT(s_callCounterName.c_str());
#endif
		// 型IDは「型の identity」なので、この値はシーンやリポジトリのインスタンスに依存しない。
		// 以前ここに置いていた static は「このリポジトリの vector の何番目か」を覚えており、
		// シーンを作り直すと古い位置を指したままになっていた。覚える対象を変えたことで static が正しくなる。
		const uint32_t typeId = GetComponentTypeId<ComponentType>();
		if(typeId >= kMaxComponentTypes){
			LOG_ERROR("ComponentRepository: type id is not available for type: {}",nameof<ComponentType>());
			return nullptr;
		}

		if(!componentArrays_[typeId]){
			// 未登録の場合はここで遅延登録する。
			// 型名指定版(.cpp)と必ず同じ挙動にしておくこと。片方だけ遅延登録を失ったことがあり、
			// AddComponent のフォールド式が nullptr を参照してアクセス違反で落ちた。
			if(!RegisterComponentArray<ComponentType>()){
				return nullptr;
			}
		}

		// ComponentArray<T> は IComponentArray の派生なので、ダウンキャストは static_cast が正しい
		return static_cast<ComponentArray<ComponentType>*>(componentArrays_[typeId].get());
	}

	template <IsComponent ComponentType>
	inline std::vector<ComponentType>& ComponentRepository::GetComponents(const EntityHandle& _handle){
		auto componentArray = GetComponentArray<ComponentType>();
		if(componentArray == nullptr){
			static std::vector<ComponentType> emptyVector;
			return emptyVector;
		}
		return componentArray->GetComponents(_handle);
	}

	template <IsComponent ComponentType>
	inline ComponentType* ComponentRepository::GetComponent(const EntityHandle& _handle,uint32_t _index){
		ComponentArray<ComponentType>* componentArray = GetComponentArray<ComponentType>();
		if(componentArray == nullptr){
			return nullptr;
		}
		return componentArray->GetComponent(_handle,_index);
	}

	template <IsComponent ComponentType>
	inline ComponentType* ComponentRepository::GetComponent(ComponentHandle _handle){
		ComponentArray<ComponentType>* componentArray = GetComponentArray<ComponentType>();
		if(componentArray == nullptr){
			return nullptr;
		}
		return componentArray->GetComponent(_handle);
	}

	template <IsComponent... ComponentType>
	inline void ComponentRepository::AddComponent(Scene* _scene,const EntityHandle& _handle){
		// フォールド式で、パラメータパック内の各型に対してAddComponentを順に呼び出す
		(this->GetComponentArray<ComponentType>()->AddComponent(_scene,_handle),...);
	}

	template <IsComponent ComponentType>
	inline void ComponentRepository::RemoveComponent(const EntityHandle& _handle){
		auto componentArray = GetComponentArray<ComponentType>();
		if(componentArray){
			// ComponentArray::RemoveComponent が内部で対象コンポーネントの Finalize() を必ず呼ぶため、
			// ここで別途終了処理を行う必要はない(_doFinalize は呼び出し側の互換のために残している)
			componentArray->RemoveComponent(_handle);
		} else{
			LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}",nameof<ComponentType>());
		}
	}

} // namespace OriGine
