#pragma once
#include "ComponentArray.h"
#include "ComponentTypeId.h"

/// DLL境界
#include "OriGineApi.h"

namespace OriGine {

/// <summary>
/// Component Registry
/// ComponentTypeは EXEにおいて一意であり,
/// Sceneで実際に使用される実体 は ComponentRepository に格納される.
/// ここが保持するのは「型名(文字列) -> その型専用のComponentArrayを作るファクトリ関数」の対応表のみで、
/// 実データは持たない。型名という文字列をキーにすることで、
/// ・シリアライズされたシーンJSONに書かれた型名から対応するComponentArrayを実行時に復元する
/// ・エディタがコンパイル時に型を知らなくても、登録済み型名の一覧からコンポーネント追加UIを構築する
/// ・ComponentRepositoryがSceneごとにComponentArray群を複製生成する
/// といった、コンパイル時の型と実行時の文字列表現とを結びつける処理をすべてここに集約できる。
/// </summary>
class ORIGINE_API ComponentRegistry final {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する
    /// </summary>
    static ComponentRegistry* GetInstance();

    /// <summary>
    /// ComponentTypeに対応するComponentArrayの複製関数を登録する
    /// </summary>
    /// <param name="_makeCloneFunc">ComponentArrayを生成するファクトリ関数(省略時はデフォルトのnew)</param>
    template <IsComponent ComponentType>
    void RegisterComponent(
        std::function<std::unique_ptr<IComponentArray>()> _makeCloneFunc =
            []() -> std::unique_ptr<IComponentArray> {
            return std::make_unique<ComponentArray<ComponentType>>();
        });

    /// <summary>
    /// 登録済みのファクトリ関数からComponentArrayを複製生成する(型指定版)
    /// </summary>
    /// <returns>生成されたComponentArray</returns>
    template <IsComponent ComponentType>
    std::unique_ptr<IComponentArray> CloneComponentArray();

    /// <summary>
    /// 登録済みのファクトリ関数からComponentArrayを複製生成する(型名指定版)
    /// </summary>
    /// <param name="_compTypeName">ComponentTypeの型名</param>
    /// <returns>生成されたComponentArray</returns>
    std::unique_ptr<IComponentArray> CloneComponentArray(const std::string& _compTypeName);

    /// <summary>
    /// 型名に対応する型IDを返す。未採番なら新たに採番する。
    /// 採番はプロセス内で一意・0始まり・密(Phase 5 のビットマスクがビット位置として使えること)。
    /// </summary>
    /// <param name="_typeName">コンポーネントの型名</param>
    /// <returns>型ID。上限に達していれば kInvalidComponentTypeId</returns>
    uint32_t AcquireTypeId(const std::string& _typeName);

    /// <summary>
    /// 型名に対応する型IDを返す。未採番でも採番しない(問い合わせだけしたい経路用)。
    /// </summary>
    /// <returns>型ID。未採番なら kInvalidComponentTypeId</returns>
    uint32_t FindTypeId(const std::string& _typeName) const;

    /// <summary>
    /// 型IDから型名を逆引きする(エディタ表示・シリアライズ用の低頻度パス)。
    /// </summary>
    /// <returns>型名。範囲外なら空文字列</returns>
    const std::string& GetTypeName(uint32_t _typeId) const;

    /// <summary>採番済みのコンポーネント型の数</summary>
    uint32_t GetTypeCount() const { return static_cast<uint32_t>(idToTypeName_.size()); }


private:
    ComponentRegistry();
    ~ComponentRegistry();
    ComponentRegistry(const ComponentRegistry&)            = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<IComponentArray>()>> cloneMaker_; // 型名 -> ComponentArray生成関数

    // 型名 <-> 型ID の対応表。型IDは「型の identity」であり、ComponentRepository の
    // vector の添字としてそのまま使われる。シーンごとの位置ではないので、シーンを作り直しても
    // 配列から要素を消しても意味が変わらない。
    //
    // 採番は実行時の呼び出し順で決まるため、この値をファイルへ書き出してはならない。
    // シリアライズは型名のままで行い、idToTypeName_ は逆引き専用に使う。
    std::unordered_map<std::string, uint32_t> typeNameToId_;
    std::vector<std::string> idToTypeName_; // 添字 = 型ID


    // ComponentRegistry 自身はコンポーネントではなくシングルトンなので、この配列を無条件にしても
    // test.ps1 -Suite layout が見ているコンポーネント型の sizeof/alignof には影響しない。
    // push_back は RegisterComponent<T>() の中(起動時に数回)でのみ呼ばれ、計測対象のフレームには乗らない。
    std::vector<std::string> componentTypeNames_; // 登録済みComponent型名の一覧(エディタ表示用)

public:
    /// <summary>
    /// 登録済みComponent型名の一覧を取得する(エディタ表示用)
    /// </summary>
    /// <returns>登録済み型名の一覧</returns>
    const std::vector<std::string>& GetComponentTypeNames() const {
        return componentTypeNames_;
    }

    /// <summary>
    /// 指定した型名のComponentArrayが登録済みか(型名指定版)
    /// </summary>
    /// <param name="_typeName">ComponentTypeの型名</param>
    /// <returns>登録済みであればtrue</returns>
    bool HasComponentArray(const std::string& _typeName) const {
        return cloneMaker_.find(_typeName) != cloneMaker_.end();
    }
    /// <summary>
    /// 指定した型のComponentArrayが登録済みか(型指定版)
    /// </summary>
    /// <returns>登録済みであればtrue</returns>
    template <IsComponent ComponentType>
    bool HasComponentArray() const {
        static std::string typeName = nameof<ComponentType>();
        return cloneMaker_.find(typeName) != cloneMaker_.end();
    }
};

template <IsComponent ComponentType>
void ComponentRegistry::RegisterComponent(
    std::function<std::unique_ptr<IComponentArray>()> _makeCloneFunc) {
    static std::string typeName = nameof<ComponentType>();
    if (cloneMaker_.find(typeName) != cloneMaker_.end()) {
        // 二重登録は上書きされるだけなので警告のみ出す
        LOG_WARN("ComponentRegistry: ComponentArray already registered for type: {}", typeName);
    }
    cloneMaker_[typeName] = _makeCloneFunc;

    // 型IDはここで確定させる。FrameWork の明示的な登録リストから1回だけ呼ばれるので採番順が決定的になり、
    // 静的初期化子による自己登録(このビルドではリンカに捨てられる)を使わずに済む。
    ComponentTypeIdStorage<ComponentType>::id_ = AcquireTypeId(typeName);


    componentTypeNames_.push_back(typeName);
}

/// <summary>
/// コンポーネント型 T の型IDを取得する(ホットパス用)。
/// 初回だけ ComponentRegistry へ問い合わせ、以降は定数初期化された static からの素のロードで済む。
/// RegisterComponent<T>() を通っていない型でも、ここで採番して以降は同じIDで扱えるようにしている
/// (テンプレート版と型名指定版で挙動が食い違うと、かつてのように片方だけ nullptr を返して落ちるため)。
/// </summary>
template <IsComponent ComponentType>
inline uint32_t GetComponentTypeId() {
    uint32_t& id = ComponentTypeIdStorage<ComponentType>::id_;
    if (id == kInvalidComponentTypeId) {
        id = ComponentRegistry::GetInstance()->AcquireTypeId(nameof<ComponentType>());
    }
    return id;
}

template <IsComponent ComponentType>
std::unique_ptr<IComponentArray> ComponentRegistry::CloneComponentArray() {
    static std::string _typeName = nameof<ComponentType>();
    auto itr              = cloneMaker_.find(_typeName);
    if (itr == cloneMaker_.end()) {
        LOG_ERROR("ComponentRegistry: Clone maker not found for type: {}", _typeName);
        return nullptr;
    }
    return itr->second();
}


} // namespace OriGine
