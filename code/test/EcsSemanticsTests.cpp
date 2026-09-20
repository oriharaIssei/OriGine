#include "test/EcsSemanticsTests.h"

/// stl
#include <cstdint>
#include <format>
#include <string>
#include <unordered_set>
#include <vector>

/// ECS
#include "component/ComponentRepository.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"
#include "entity/EntityRepository.h"
#include "system/ISystem.h"

/// externals
#include <nlohmann/json.hpp>

using namespace OriGine;

namespace OriGine::Test {

namespace {

/// <summary>
/// ケース8(SystemEntityRegistration)専用の、何もしないISystem実装。
/// AddEntity/RemoveEntity/HasEntity/ClearEntitiesはISystem基底のインラインメンバであり
/// scene_/entityRepository_に一切触れないため、Sceneを持たないこの空実装だけで検証できる。
/// </summary>
class NullTestSystem final : public ISystem {
public:
    NullTestSystem() : ISystem(SystemCategory::Movement) {}
    void Initialize() override {}
    void Finalize() override {}
};

} // namespace

/// <summary>
/// ケース1: エンティティを作ったら有効で、壊したら無効になる。
/// なぜ固定するか: 「生成したハンドルは使える」「破棄したハンドルは使えない」はストレージの
/// 実装(UUID方式でもindex+generation方式でも)を問わず成り立つべき、ECSの最も基本的な契約。
/// </summary>
TestCaseResult EntityLifecycle_CreateAndDestroy() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository repo;
    repo.Initialize();

    EntityHandle handle = repo.CreateEntity("TestEntity");

    bool validAfterCreate   = handle.IsValid();
    bool aliveAfterCreate   = repo.IsAlive(handle);
    Entity* entityAfterCreate = repo.GetEntity(handle);
    bool getOkAfterCreate   = (entityAfterCreate != nullptr) && entityAfterCreate->IsAlive();

    bool createOk = validAfterCreate && aliveAfterCreate && getOkAfterCreate;
    result.passed &= createOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] create: valid={} alive={} get!=null&&IsAlive={}",
        createOk ? "ok  " : "FAIL", validAfterCreate, aliveAfterCreate, getOkAfterCreate));

    bool removed = repo.RemoveEntity(handle);
    bool aliveAfterRemove = repo.IsAlive(handle);
    Entity* entityAfterRemove = repo.GetEntity(handle);

    bool removeOk = removed && !aliveAfterRemove && (entityAfterRemove == nullptr);
    result.passed &= removeOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] destroy: removed={} (expected true) alive={} (expected false) get==null={} (expected true)",
        removeOk ? "ok  " : "FAIL", removed, aliveAfterRemove, entityAfterRemove == nullptr));

    // 二重破棄は「もう一度消せてしまう」のではなく失敗として報告されるべき
    // (Phase5でスロットが再利用されている場合、二重破棄を黙認すると
    // 無関係な別Entityを巻き込んで消してしまう事故につながる)。
    bool secondRemove = repo.RemoveEntity(handle);
    bool doubleRemoveOk = !secondRemove;
    result.passed &= doubleRemoveOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] double destroy: second RemoveEntity={} (expected false)",
        doubleRemoveOk ? "ok  " : "FAIL", secondRemove));

    return result;
}

/// <summary>
/// ケース2: 破棄したエンティティのハンドルで、後から生成した別のエンティティが取れてしまわない。
/// なぜ固定するか: これはPhase 5でgenerationカウンタが担う役割そのもの。今のUUID実装でも
/// 「破棄されたスロットが再利用されても、古いUUIDは新しいEntityを指さない」という形で
/// 同じ契約が成り立っているはずなので、書き換え後も崩れてはならない一番大事な不変条件。
/// </summary>
TestCaseResult HandleReuse_NoAliasing() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository repo;
    repo.Initialize();

    EntityHandle handleA = repo.CreateEntity("A");
    bool removedA = repo.RemoveEntity(handleA);
    // Aが空けたスロットをBが再利用する可能性が高い実装(空きビットの先頭から確保する)だが、
    // 再利用してもしなくてもこの先の判定式自体は成り立つべきなので、ここでは前提にしない。
    EntityHandle handleB = repo.CreateEntity("B");

    bool distinctHandles = (handleA != handleB);
    bool aAliveNow = repo.IsAlive(handleA);
    bool bAliveNow = repo.IsAlive(handleB);
    Entity* viaA = repo.GetEntity(handleA);
    Entity* viaB = repo.GetEntity(handleB);
    bool bResolvesToB = (viaB != nullptr) && (viaB->GetHandle() == handleB);

    bool ok = removedA && distinctHandles && !aAliveNow && bAliveNow && (viaA == nullptr) && bResolvesToB;
    result.passed &= ok;
    result.diagnosticLines.push_back(std::format(
        "[{}] single reuse: removedA={} distinctHandles={} aAlive={} (expected false) bAlive={} (expected true) "
        "viaA==null={} (expected true) viaB resolves to B={} (expected true)",
        ok ? "ok  " : "FAIL", removedA, distinctHandles, aAliveNow, bAliveNow, viaA == nullptr, bResolvesToB));

    return result;
}

/// <summary>
/// ケース3: EntityHandleをJSONへ往復させても、指しているエンティティの同一性が保たれる。
/// なぜ固定するか: docs/todo.htmlのPhase 5計画は「既存セーブデータとの互換レイヤ(UUIDは
/// シリアライズ層のみに残す)」を明記している。つまりEntityHandleの内部表現が
/// {index,generation}に変わっても、JSONとしてはUUID相当の値をやり取りし続け、
/// 「同じJSONから復元したハンドルは同じエンティティを指す」という契約は保たれる想定。
/// serialize-goldenスイートはコンポーネントの値そのものの往復を見ており、
/// エンティティの識別子としての往復は見ていないため、ここで別途固定する。
/// </summary>
TestCaseResult EntityHandle_JsonRoundTrip_PreservesIdentity() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository repo;
    repo.Initialize();

    EntityHandle original = repo.CreateEntity("RoundTripTarget");

    nlohmann::json json = original; // to_json(ADL)
    EntityHandle restored;
    json.get_to(restored); // from_json(ADL)

    bool equalHandles = (restored == original);
    bool restoredAlive = repo.IsAlive(restored);
    Entity* viaOriginal = repo.GetEntity(original);
    Entity* viaRestored = repo.GetEntity(restored);
    bool samePointee = (viaOriginal != nullptr) && (viaOriginal == viaRestored);

    bool ok = equalHandles && restoredAlive && samePointee;
    result.passed &= ok;
    result.diagnosticLines.push_back(std::format(
        "[{}] json roundtrip: equalHandles={} restoredAlive={} samePointee={} json={}",
        ok ? "ok  " : "FAIL", equalHandles, restoredAlive, samePointee, json.dump()));

    return result;
}

/// <summary>
/// ケース4: 数千体のエンティティを作って消して作り直しても、生存状態とハンドルの一意性が崩れない。
/// なぜ固定するか: Phase 5はストリーミング(ワールドの出入りに伴う常時の生成・破棄)が前提。
/// docs/todo.htmlの「前提」に明記の通り、ここは性能問題ではなくオープンワールドの前提条件
/// そのものであり、数千回の生成・破棄を経ても意味論(生きているものは生きている、
/// 消したものは消えている、誰も他人のハンドルを名乗らない)が壊れないことを保証する。
/// </summary>
TestCaseResult ManyEntities_CreateDestroyRecreate() {
    TestCaseResult result;
    result.passed = true;

    constexpr size_t kInitialCount = 3000;

    EntityRepository repo;
    repo.Initialize();

    std::vector<EntityHandle> allHandlesEverIssued;
    allHandlesEverIssued.reserve(kInitialCount * 2);

    std::vector<EntityHandle> initial;
    initial.reserve(kInitialCount);
    for (size_t i = 0; i < kInitialCount; ++i) {
        EntityHandle h = repo.CreateEntity("Churn");
        initial.push_back(h);
        allHandlesEverIssued.push_back(h);
    }

    bool countAfterCreateOk = (repo.GetEntityCount() == kInitialCount);

    // 半分(偶数番目)を破棄する
    std::vector<EntityHandle> destroyed;
    std::vector<EntityHandle> survivors;
    destroyed.reserve(kInitialCount / 2);
    survivors.reserve(kInitialCount / 2);
    for (size_t i = 0; i < initial.size(); ++i) {
        if (i % 2 == 0) {
            repo.RemoveEntity(initial[i]);
            destroyed.push_back(initial[i]);
        } else {
            survivors.push_back(initial[i]);
        }
    }

    // 破棄した分と同じ数だけ新規に作り直す(空いたスロットの再利用を誘発する)
    std::vector<EntityHandle> recreated;
    recreated.reserve(destroyed.size());
    for (size_t i = 0; i < destroyed.size(); ++i) {
        EntityHandle h = repo.CreateEntity("Recreated");
        recreated.push_back(h);
        allHandlesEverIssued.push_back(h);
    }

    // 生存数は survivors + recreated のはず
    size_t expectedAliveCount = survivors.size() + recreated.size();
    bool countAfterChurnOk = (repo.GetEntityCount() == expectedAliveCount);

    // 破棄したハンドルは全て死んでいる
    bool allDestroyedDead = true;
    for (const EntityHandle& h : destroyed) {
        if (repo.IsAlive(h) || repo.GetEntity(h) != nullptr) {
            allDestroyedDead = false;
            break;
        }
    }

    // 生き残り + 作り直した分は全て生きていて、自分自身を指す
    bool allAliveResolveToSelf = true;
    for (const EntityHandle& h : survivors) {
        Entity* e = repo.GetEntity(h);
        if (!repo.IsAlive(h) || e == nullptr || e->GetHandle() != h) {
            allAliveResolveToSelf = false;
            break;
        }
    }
    for (const EntityHandle& h : recreated) {
        Entity* e = repo.GetEntity(h);
        if (!repo.IsAlive(h) || e == nullptr || e->GetHandle() != h) {
            allAliveResolveToSelf = false;
            break;
        }
    }

    // これまで発行された全ハンドルの中に重複が無い(=スロット再利用が別人のなりすましを
    // 生んでいない)。EntityHandleはunordered_map対応のためのstd::hash特殊化を持つので
    // そのままunordered_setに積める。
    std::unordered_set<EntityHandle> uniqueCheck;
    bool noDuplicateHandles = true;
    for (const EntityHandle& h : allHandlesEverIssued) {
        if (!uniqueCheck.insert(h).second) {
            noDuplicateHandles = false;
            break;
        }
    }

    bool ok = countAfterCreateOk && countAfterChurnOk && allDestroyedDead && allAliveResolveToSelf && noDuplicateHandles;
    result.passed &= ok;
    result.diagnosticLines.push_back(std::format(
        "[{}] churn {} initial, destroy {}, recreate {}: countAfterCreate={} (expected {}) "
        "countAfterChurn={} (expected {}) allDestroyedDead={} allAliveResolveToSelf={} noDuplicateHandles={}",
        ok ? "ok  " : "FAIL", kInitialCount, destroyed.size(), recreated.size(),
        repo.GetEntityCount(), kInitialCount, // (create直後の値は既に消費済みなので参考表示。実測はcountAfterCreateOkで確認済み)
        expectedAliveCount, expectedAliveCount, allDestroyedDead, allAliveResolveToSelf, noDuplicateHandles));

    return result;
}

/// <summary>
/// ケース5: コンポーネントを付ける前は取れない、付けたら取れる、消したら取れない、
/// 付けていない型は取れない。
/// なぜ固定するか: ECSの最も基本的な入出力契約。ストレージがunordered_mapベースでも
/// アーキタイプ/スパースセットベースでも「付けた値が返ってくる」ことだけは変わってはならない。
/// </summary>
TestCaseResult ComponentAddGetRemove_SingleType() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository entityRepo;
    entityRepo.Initialize();
    EntityHandle handle = entityRepo.CreateEntity("HasTransform");

    ComponentRepository compRepo;

    Transform* beforeAdd = compRepo.GetComponent<Transform>(handle);
    bool beforeOk = (beforeAdd == nullptr);
    result.passed &= beforeOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] before AddComponent: GetComponent<Transform>==null={} (expected true)",
        beforeOk ? "ok  " : "FAIL", beforeAdd == nullptr));

    ComponentHandle compHandle = compRepo.GetComponentArray<Transform>()->AddComponent(nullptr, handle);
    Transform* afterAdd = compRepo.GetComponent<Transform>(handle);
    bool addOk = (afterAdd != nullptr) && compHandle.IsValid();
    result.passed &= addOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] AddComponent: GetComponent!=null={} compHandle valid={}",
        addOk ? "ok  " : "FAIL", afterAdd != nullptr, compHandle.IsValid()));

    if (afterAdd) {
        afterAdd->translate = Vec3f{1.0f, 2.0f, 3.0f};
    }

    Transform* readBack = compRepo.GetComponent<Transform>(handle);
    Transform* readBackByHandle = compRepo.GetComponent<Transform>(compHandle);
    bool valueOk = (readBack != nullptr) && (readBack->translate == Vec3f{1.0f, 2.0f, 3.0f});
    bool sameEntityAndHandlePathAgree = (readBack == readBackByHandle);
    result.passed &= valueOk && sameEntityAndHandlePathAgree;
    result.diagnosticLines.push_back(std::format(
        "[{}] set then get: translate matches set value={} EntityHandle/ComponentHandle paths agree={}",
        (valueOk && sameEntityAndHandlePathAgree) ? "ok  " : "FAIL", valueOk, sameEntityAndHandlePathAgree));

    compRepo.RemoveComponent<Transform>(handle);
    Transform* afterRemove = compRepo.GetComponent<Transform>(handle);
    bool removeOk = (afterRemove == nullptr);
    result.passed &= removeOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] RemoveComponent: GetComponent==null={} (expected true)",
        removeOk ? "ok  " : "FAIL", afterRemove == nullptr));

    // 一度もAddしていない型(Rigidbody)はこのエンティティからは取れない
    Rigidbody* neverAdded = compRepo.GetComponent<Rigidbody>(handle);
    bool neverAddedOk = (neverAdded == nullptr);
    result.passed &= neverAddedOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] type never added to this entity: GetComponent<Rigidbody>==null={} (expected true)",
        neverAddedOk ? "ok  " : "FAIL", neverAdded == nullptr));

    return result;
}

/// <summary>
/// ケース6: 1エンティティに複数の型(Transform, Rigidbody)を付けたとき、それぞれ独立に
/// 値を持ち、片方を消してももう片方は無事であること。
/// なぜ固定するか: アーキタイプ/スパースセットどちらを選んでも「型ごとの記憶域は互いに独立」
/// という前提は変わらない。ここが崩れると、片方の型を書き換えたら別の型の値まで
/// 化けるという最悪のバグになる。
/// </summary>
TestCaseResult MultipleComponentTypes_Independent() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository entityRepo;
    entityRepo.Initialize();
    EntityHandle handle = entityRepo.CreateEntity("MultiType");

    ComponentRepository compRepo;
    compRepo.GetComponentArray<Transform>()->AddComponent(nullptr, handle);
    compRepo.GetComponentArray<Rigidbody>()->AddComponent(nullptr, handle);

    Transform* t = compRepo.GetComponent<Transform>(handle);
    Rigidbody* rb = compRepo.GetComponent<Rigidbody>(handle);
    bool bothAdded = (t != nullptr) && (rb != nullptr);
    result.passed &= bothAdded;
    result.diagnosticLines.push_back(std::format(
        "[{}] both types added: Transform!=null={} Rigidbody!=null={}",
        bothAdded ? "ok  " : "FAIL", t != nullptr, rb != nullptr));

    if (t) {
        t->translate = Vec3f{10.0f, 20.0f, 30.0f};
    }
    if (rb) {
        rb->SetPrePos(Vec3f{40.0f, 50.0f, 60.0f});
    }

    Transform* t2 = compRepo.GetComponent<Transform>(handle);
    Rigidbody* rb2 = compRepo.GetComponent<Rigidbody>(handle);
    bool independentOk = (t2 != nullptr) && (rb2 != nullptr)
                          && (t2->translate == Vec3f{10.0f, 20.0f, 30.0f})
                          && (rb2->GetPrePos() == Vec3f{40.0f, 50.0f, 60.0f});
    result.passed &= independentOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] independent values: Transform.translate matches={} Rigidbody.prePos matches={}",
        independentOk ? "ok  " : "FAIL",
        t2 && t2->translate == Vec3f{10.0f, 20.0f, 30.0f},
        rb2 && rb2->GetPrePos() == Vec3f{40.0f, 50.0f, 60.0f}));

    // Rigidbodyだけ消してもTransformは無事
    compRepo.RemoveComponent<Rigidbody>(handle);
    Rigidbody* rbAfterRemove = compRepo.GetComponent<Rigidbody>(handle);
    Transform* tAfterRigidbodyRemoved = compRepo.GetComponent<Transform>(handle);
    bool survivalOk = (rbAfterRemove == nullptr)
                       && (tAfterRigidbodyRemoved != nullptr)
                       && (tAfterRigidbodyRemoved->translate == Vec3f{10.0f, 20.0f, 30.0f});
    result.passed &= survivalOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] remove Rigidbody only: Rigidbody gone={} Transform still intact with same value={}",
        survivalOk ? "ok  " : "FAIL", rbAfterRemove == nullptr, survivalOk));

    return result;
}

/// <summary>
/// ケース7: 今の実装は同一エンティティに同じ型のコンポーネントを複数持てる(EntitySlot::componentsが
/// vector)。この現状の振る舞いをそのまま固定する。
/// なぜ固定するか: docs/todo.htmlのPhase 5「未決」に「同一型コンポーネントの多重所有」が
/// 挙げられている通り、ここはユーザーがPhase 5で「捨てる」か「例外テーブルにする」かを
/// 決める対象そのもの。捨てると決めたら、このテストケースは(このコメントごと)削除してよい
/// ―― 削除することが正しい判断であり、事故ではない。逆に、決定前にこの振る舞いが
/// 意図せず壊れたら(例えば2つ目を追加したらクラッシュする等)、それは事故として扱う。
/// </summary>
TestCaseResult DuplicateComponentOwnership_CurrentBehavior() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository entityRepo;
    entityRepo.Initialize();
    EntityHandle handle = entityRepo.CreateEntity("DuplicateOwner");

    ComponentRepository compRepo;
    ComponentArray<Transform>* arr = compRepo.GetComponentArray<Transform>();

    arr->AddComponent(nullptr, handle);
    arr->AddComponent(nullptr, handle);

    uint32_t countAfterTwoAdds = arr->GetComponentCount(handle);
    bool twoAddsOk = (countAfterTwoAdds == 2);
    result.passed &= twoAddsOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] add Transform twice to same entity: count={} (expected 2, current impl allows multi-ownership)",
        twoAddsOk ? "ok  " : "FAIL", countAfterTwoAdds));

    Transform* slot0 = arr->GetComponent(handle, 0);
    Transform* slot1 = arr->GetComponent(handle, 1);
    bool bothSlotsOk = (slot0 != nullptr) && (slot1 != nullptr) && (slot0 != slot1);
    if (slot0) slot0->translate = Vec3f{1.0f, 0.0f, 0.0f};
    if (slot1) slot1->translate = Vec3f{2.0f, 0.0f, 0.0f};

    bool indexedIndependence = bothSlotsOk
                                && (arr->GetComponent(handle, 0)->translate == Vec3f{1.0f, 0.0f, 0.0f})
                                && (arr->GetComponent(handle, 1)->translate == Vec3f{2.0f, 0.0f, 0.0f});
    result.passed &= indexedIndependence;
    result.diagnosticLines.push_back(std::format(
        "[{}] index-addressed slots are independent: slot0!=slot1={} slot0==(1,0,0)&&slot1==(2,0,0)={}",
        indexedIndependence ? "ok  " : "FAIL", bothSlotsOk, indexedIndependence));

    // index 0 を消すと、元index 1が詰められてindex 0に来る(現在の実装の挙動)。
    // これは「vectorから消したら詰まる」という内部実装ではなく、GetComponent(handle, index)
    // という公開APIの引数であるindexが指す意味の話なので、公開契約として観測できる。
    arr->RemoveComponent(handle, 0);
    uint32_t countAfterRemove = arr->GetComponentCount(handle);
    Transform* remaining = arr->GetComponent(handle, 0);
    bool compactionOk = (countAfterRemove == 1) && remaining && (remaining->translate == Vec3f{2.0f, 0.0f, 0.0f});
    result.passed &= compactionOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] remove index 0: count={} (expected 1) remaining index0 == what was index1 (2,0,0)={}",
        compactionOk ? "ok  " : "FAIL", countAfterRemove, compactionOk));

    return result;
}

/// <summary>
/// ケース8: EntityRepositoryとComponentRepositoryをまたいだ破棄の組み合わせ。
/// Scene::ExecuteDeleteEntitiesは「コンポーネント削除 → システム登録解除 → Entity実体削除」の
/// 順で呼ぶ(ComponentRepository::RemoveEntityのコメント参照)。ここではScene無しで
/// 同じ組み合わせを再現し、両方のリポジトリをまたいだ後にも意味論が保たれることを確認する。
/// なぜ固定するか: 2つの別々のストレージ(Entity用/Component用)が今後どう統合・分離されても、
/// 「エンティティを壊したら、そのエンティティに付いていたコンポーネントも取れなくなる」という
/// 利用者から見た結果は変わってはならない。
/// </summary>
TestCaseResult EntityAndComponentLifecycle_Combined() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository entityRepo;
    entityRepo.Initialize();
    EntityHandle handle = entityRepo.CreateEntity("CombinedLifecycle");

    ComponentRepository compRepo;
    compRepo.GetComponentArray<Transform>()->AddComponent(nullptr, handle);

    bool addedOk = (compRepo.GetComponent<Transform>(handle) != nullptr);
    result.passed &= addedOk;

    // Sceneが行う順序を模す: まずコンポーネントを全削除、それからEntity実体を削除
    compRepo.RemoveEntity(handle);
    bool componentGoneAfterCompRemove = (compRepo.GetComponent<Transform>(handle) == nullptr);

    bool entityRemoved = entityRepo.RemoveEntity(handle);
    bool entityGone = !entityRepo.IsAlive(handle) && (entityRepo.GetEntity(handle) == nullptr);
    // Entity実体を消した後でも、(既に空になっている)コンポーネント側の再取得はnullptrのままのはず
    bool componentStillGoneAfterEntityRemove = (compRepo.GetComponent<Transform>(handle) == nullptr);

    bool ok = addedOk && componentGoneAfterCompRemove && entityRemoved && entityGone && componentStillGoneAfterEntityRemove;
    result.passed &= ok;
    result.diagnosticLines.push_back(std::format(
        "[{}] combined teardown: addedOk={} componentGoneAfterCompRemove={} entityRemoved={} entityGone={} "
        "componentStillGoneAfterEntityRemove={}",
        ok ? "ok  " : "FAIL", addedOk, componentGoneAfterCompRemove, entityRemoved, entityGone,
        componentStillGoneAfterEntityRemove));

    return result;
}

/// <summary>
/// ケース9: エンティティをシステムに登録・解除したときの挙動(重複登録の無視、解除、全解除)。
/// なぜ固定するか: ISystem::AddEntity/RemoveEntity/HasEntityはPhase 5のロードマップにある
/// 「ISystem::AddEntityの線形探索O(N^2)を解消する」の対象そのもの。中身がstd::vectorの
/// 線形探索からハッシュ集合やビットセットに変わっても、「同じエンティティを二重登録しない」
/// 「解除したら見えなくなる」という利用者から見た契約は変わってはならない。
/// </summary>
TestCaseResult SystemEntityRegistration_AddRemoveClear() {
    TestCaseResult result;
    result.passed = true;

    EntityRepository entityRepo;
    entityRepo.Initialize();
    EntityHandle handleA = entityRepo.CreateEntity("SysA");
    EntityHandle handleB = entityRepo.CreateEntity("SysB");

    NullTestSystem system;

    system.AddEntity(handleA);
    bool afterFirstAdd = system.HasEntity(handleA) && (system.GetEntityCount() == 1);
    result.passed &= afterFirstAdd;

    // 同じハンドルの重複登録は無視される
    system.AddEntity(handleA);
    bool afterDuplicateAdd = (system.GetEntityCount() == 1);
    result.passed &= afterDuplicateAdd;

    system.AddEntity(handleB);
    bool afterSecondAdd = system.HasEntity(handleA) && system.HasEntity(handleB) && (system.GetEntityCount() == 2);
    result.passed &= afterSecondAdd;

    system.RemoveEntity(handleA);
    bool afterRemoveA = !system.HasEntity(handleA) && system.HasEntity(handleB) && (system.GetEntityCount() == 1);
    result.passed &= afterRemoveA;

    system.ClearEntities();
    bool afterClear = !system.HasEntity(handleA) && !system.HasEntity(handleB) && (system.GetEntityCount() == 0);
    result.passed &= afterClear;

    bool ok = afterFirstAdd && afterDuplicateAdd && afterSecondAdd && afterRemoveA && afterClear;
    result.diagnosticLines.push_back(std::format(
        "[{}] add/duplicate-add/add/remove/clear: firstAdd={} duplicateIgnored={} secondAdd={} removeA={} clear={}",
        ok ? "ok  " : "FAIL", afterFirstAdd, afterDuplicateAdd, afterSecondAdd, afterRemoveA, afterClear));

    return result;
}

std::vector<TestCaseEntry> MakeEcsSemanticsTestCases() {
    return {
        {"EntityLifecycle_CreateAndDestroy", EntityLifecycle_CreateAndDestroy},
        {"HandleReuse_NoAliasing", HandleReuse_NoAliasing},
        {"EntityHandle_JsonRoundTrip_PreservesIdentity", EntityHandle_JsonRoundTrip_PreservesIdentity},
        {"ManyEntities_CreateDestroyRecreate", ManyEntities_CreateDestroyRecreate},
        {"ComponentAddGetRemove_SingleType", ComponentAddGetRemove_SingleType},
        {"MultipleComponentTypes_Independent", MultipleComponentTypes_Independent},
        {"DuplicateComponentOwnership_CurrentBehavior", DuplicateComponentOwnership_CurrentBehavior},
        {"EntityAndComponentLifecycle_Combined", EntityAndComponentLifecycle_Combined},
        {"SystemEntityRegistration_AddRemoveClear", SystemEntityRegistration_AddRemoveClear},
    };
}

} // namespace OriGine::Test
