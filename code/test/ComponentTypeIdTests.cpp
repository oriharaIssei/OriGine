#include "test/ComponentTypeIdTests.h"

/// stl
#include <cstdint>
#include <format>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

/// ECS
#include "component/ComponentRepository.h"
#include "component/collision/CollisionPushBackInfo.h"
#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/CapsuleCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/RayCollider.h"
#include "component/collision/collider/SegmentCollider.h"
#include "component/collision/collider/SphereCollider.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

/// util
#include <util/nameof.h>

using namespace OriGine;

namespace OriGine::Test {

namespace {

/// <summary>
/// ポインタを "0x0000abcd1234" 形式の文字列に変換する。
/// std::format の組み込みポインタ書式(P0645)に頼らず uintptr_t 経由で整形することで、
/// 標準ライブラリのポインタ書式サポート有無に関係なく確実に人間が読める16進表記にする。
/// </summary>
std::string PtrToString(const void* _ptr) {
    return std::format("{:#014x}", reinterpret_cast<std::uintptr_t>(_ptr));
}

/// <summary>
/// 判定式 GetComponentArray&lt;T&gt;() == GetComponentArray(nameof&lt;T&gt;()) を1型分だけ検証し、
/// 結果を _result に積む。オラクルを非テンプレート版にできる理由は ComponentTypeIdTests.h の
/// コメントを参照。テンプレート版の戻り値は ComponentArray&lt;T&gt;* 、オラクル側は
/// IComponentArray* なので、IComponentArray* に揃えてから比較する
/// (ComponentArray&lt;T&gt; は IComponentArray を単一継承しているため static_cast で安全に上位変換できる)。
/// </summary>
template <IsComponent T>
void CheckMatchesOracle(ComponentRepository& _repo, TestCaseResult& _result) {
    ComponentArray<T>* fromTemplate     = _repo.GetComponentArray<T>();
    IComponentArray* fromOracle         = _repo.GetComponentArray(nameof<T>());
    IComponentArray* fromTemplateAsBase = static_cast<IComponentArray*>(fromTemplate);

    if (fromTemplateAsBase == fromOracle) {
        _result.diagnosticLines.push_back(std::format(
            "[ok]   {:<22} template={} oracle={}",
            nameof<T>(), PtrToString(fromTemplateAsBase), PtrToString(fromOracle)));
    } else {
        _result.passed = false;
        _result.diagnosticLines.push_back(std::format(
            "[FAIL] {:<22} template={} oracle={}  (expected == , got !=)",
            nameof<T>(), PtrToString(fromTemplateAsBase), PtrToString(fromOracle)));
    }
}

/// <summary>
/// ケース1とケース4で使い回す3型を、常に同じ順序(Transform, Rigidbody, SphereCollider)で登録する。
/// 順序を揃えているのは、テストスイートの先頭で必ず実行されるケース1がこの順序で
/// static キャッシュ(添字0,1,2)を確定させ、ケース4はそのキャッシュが
/// 「Unregisterするまでは自分自身の登録内容とも一致している」という前提から出発できるようにするため。
/// こうすることでケース4は「登録順の違い」(ケース2/3のテーマ)とは無関係に、
/// UnregisterComponentArray だけが引き金になっていることを切り分けて示せる。
/// </summary>
void RegisterSharedThreeTypes(ComponentRepository& _repo) {
    _repo.RegisterComponentArray<Transform>();
    _repo.RegisterComponentArray<Rigidbody>();
    _repo.RegisterComponentArray<SphereCollider>();
}

} // namespace

/// <summary>
/// ケース1: 1つの ComponentRepository に数種類登録し、全型で判定式が成り立つことを確認する。
/// テストスイートの中で必ず最初に実行されるため、ここで使う3型(Transform/Rigidbody/
/// SphereCollider)にとってこれがプロセス全体で最初の GetComponentArray&lt;T&gt;() 呼び出しになる。
/// static キャッシュは「未設定 -> このrepoの添字で確定」という一番素直な初回登録の流れしか
/// 経験していないので、今日もPASSするはず。ここが落ちるならテスト自体かオラクルの理解が
/// 間違っているので、まずそちらを疑うこと。
/// </summary>
TestCaseResult SameRepository_TemplateMatchesStringLookup() {
    TestCaseResult result;

    ComponentRepository repo;
    RegisterSharedThreeTypes(repo);

    CheckMatchesOracle<Transform>(repo, result);
    CheckMatchesOracle<Rigidbody>(repo, result);
    CheckMatchesOracle<SphereCollider>(repo, result);

    return result;
}

/// <summary>
/// ケース2: 2つの ComponentRepository に同じ3型を逆順で登録し、両方で判定式を確認する。
/// (FAIL期待)
///
/// ケース1が使った3型とは重ならない、ここで初めて触れる3型(AABB/OBB/CapsuleCollider)を使う。
/// これはstatic キャッシュが「最初にGetComponentArray&lt;T&gt;()を呼んだ場所」で確定してしまう
/// バグそのものを検証したいので、ケース1の残り香(既に確定したキャッシュ)が混ざらないようにする
/// ための意図的な選択であり、9種の型プールをケースごとに使い分けている理由でもある。
///
/// AとBで登録する型の個数を必ず同じ(3個)にする: 個数が違うと、キャッシュされた添字が
/// (要素数の少ない側の)componentArrays_の範囲外を指してしまい、「別配列を返した」という
/// 検出可能な壊れ方ではなく、いきなり未定義動作のクラッシュになってしまうため。
///
/// 期待される結果: Aを先に問い合わせるので、3型ともAの並び(AABB=0,OBB=1,Capsule=2)で
/// static キャッシュが確定し、Aはその場で作った自分自身のキャッシュと比較するだけなので
/// 必ずPASSする。Bは登録順が逆(Capsule=0,OBB=1,AABB=2)なのに、参照される添字はAのまま
/// 使われるため、AABBとCapsuleは食い違う。OBBだけは3要素の逆順で中央の位置が変わらない
/// (0,1,2 を逆にしても中央の1は1のまま)という並びの都合上、偶然一致してしまう
/// ―― これはテストの見落としではなく、3要素反転の数学的な必然であり、
/// 後述のケース3でも同じ現象が起きる。
/// </summary>
TestCaseResult TwoRepositories_DifferentRegistrationOrder() {
    TestCaseResult result;

    ComponentRepository repoA;
    repoA.RegisterComponentArray<AABBCollider>();
    repoA.RegisterComponentArray<OBBCollider>();
    repoA.RegisterComponentArray<CapsuleCollider>();

    ComponentRepository repoB;
    repoB.RegisterComponentArray<CapsuleCollider>();
    repoB.RegisterComponentArray<OBBCollider>();
    repoB.RegisterComponentArray<AABBCollider>();

    result.diagnosticLines.push_back("-- repository A: registered AABB, OBB, Capsule (forward order) --");
    CheckMatchesOracle<AABBCollider>(repoA, result);
    CheckMatchesOracle<OBBCollider>(repoA, result);
    CheckMatchesOracle<CapsuleCollider>(repoA, result);

    result.diagnosticLines.push_back("-- repository B: registered Capsule, OBB, AABB (reverse order) --");
    CheckMatchesOracle<AABBCollider>(repoB, result);
    CheckMatchesOracle<OBBCollider>(repoB, result);
    CheckMatchesOracle<CapsuleCollider>(repoB, result);

    return result;
}

/// <summary>
/// ケース3: 1つの ComponentRepository で 登録 -> Clear() -> 逆順で再登録 を行い、判定式を確認する。
/// (FAIL期待)
///
/// Scene.cpp:138 (Scene::Finalize が componentRepository_->Clear() を呼ぶ経路)を模している。
/// ケース1/2とは重ならない残り3型(Segment/Ray/CollisionPushBackInfo)を使う。
///
/// Clear() の直前に一度 GetComponentArray&lt;T&gt;() を呼んでおく点が重要: これを呼ばないと、
/// このプロセスでこれら3型に初めて触れるのが「Clear後・逆順再登録後」になってしまい、
/// static キャッシュが未設定の状態から素直にマップを引いて偶然PASSしてしまう
/// (=Clear()がキャッシュを戻さないというバグを再現しないテストになる)。
/// 事前に一度呼ぶことで、static キャッシュを「Clear前の並び(Seg=0,Ray=1,Push=2)」で
/// 確定させてから、Clear() をまたいで本題の検証に入れる。
///
/// 型ID化以前は、Clear() が componentArrays_ と型名->添字の対応表を空にしても、
/// GetComponentArray&lt;T&gt;() 側の static typeIndex は Clear() の存在を知る術がなく
/// 一切戻らなかった。現在の static が覚えているのは型IDなので、Clear() で無効化されるものが無い。
///
/// 期待される結果: 再登録後の実際の並びは Push=0,Ray=1,Seg=2。static キャッシュは
/// Clear前の Seg=0,Ray=1,Push=2 のまま。SegmentColliderとCollisionPushBackInfoは
/// 位置が入れ替わるので食い違う。Rayはケース2のOBBと同じ理由(3要素反転で中央は不変)で
/// 偶然一致する。
/// </summary>
TestCaseResult AfterClear_ReRegisterInDifferentOrder() {
    TestCaseResult result;

    ComponentRepository repo;
    repo.RegisterComponentArray<SegmentCollider>();
    repo.RegisterComponentArray<RayCollider>();
    repo.RegisterComponentArray<CollisionPushBackInfo>();

    result.diagnosticLines.push_back("-- before Clear(): registered Segment, Ray, CollisionPushBackInfo (priming the static cache) --");
    CheckMatchesOracle<SegmentCollider>(repo, result);
    CheckMatchesOracle<RayCollider>(repo, result);
    CheckMatchesOracle<CollisionPushBackInfo>(repo, result);

    repo.Clear();

    repo.RegisterComponentArray<CollisionPushBackInfo>();
    repo.RegisterComponentArray<RayCollider>();
    repo.RegisterComponentArray<SegmentCollider>();

    result.diagnosticLines.push_back("-- after Clear() + re-register in reverse order (CollisionPushBackInfo, Ray, Segment) --");
    CheckMatchesOracle<SegmentCollider>(repo, result);
    CheckMatchesOracle<RayCollider>(repo, result);
    CheckMatchesOracle<CollisionPushBackInfo>(repo, result);

    return result;
}

/// <summary>
/// ケース5: 利用可能な全型(9種)を1つの ComponentRepository に登録し、
/// 各型の型ID(ComponentRegistry の採番)が「全て相異なる」かつ「全て64未満」であることを確認する。
/// (PASS期待)
///
/// 64はユーザーが決めた当面の型数上限(Phase 5でコンポーネント種別のビットマスクを
/// uint64_t 1本に収める前提)。ここでは採番された型IDだけを見ており、
/// 実際に配列を引く GetComponentArray&lt;T&gt;() は
/// 一度も呼ばない。そのため他のケースのキャッシュ汚染とは無関係で、実行順序を問わず
/// 結果が変わらない(このスイートの実行順が「1,2,3,5,4」になっている=ケース番号どおりでない
/// 理由の一部はここにある。詳細は ComponentTypeIdTests.h を参照)。
/// 診断のため、観測した「型名 -> 添字」の対応を全件出力する。
/// </summary>
TestCaseResult TypeIndicesAreDistinctAndWithinCap() {
    TestCaseResult result;

    constexpr int32_t kBitmaskCap        = 64; // Phase5でuint64_t 1本にビットマスクを収める前提の上限
    constexpr size_t kExpectedTypeCount  = 9;  // FrameWork.cpp:RegisterUsingComponents() で登録される型の数

    ComponentRepository repo;
    repo.RegisterComponentArray<Transform>();
    repo.RegisterComponentArray<Rigidbody>();
    repo.RegisterComponentArray<SphereCollider>();
    repo.RegisterComponentArray<AABBCollider>();
    repo.RegisterComponentArray<OBBCollider>();
    repo.RegisterComponentArray<CapsuleCollider>();
    repo.RegisterComponentArray<SegmentCollider>();
    repo.RegisterComponentArray<RayCollider>();
    repo.RegisterComponentArray<CollisionPushBackInfo>();

    // 型IDは ComponentRegistry がプロセス全体で採番するようになったため、
    // リポジトリごとの「型名 -> 添字」マップは存在しない。登録した型そのものから型IDを引く。
    const std::vector<std::pair<std::string, uint32_t>> indexMap = {
        {nameof<Transform>(), GetComponentTypeId<Transform>()},
        {nameof<Rigidbody>(), GetComponentTypeId<Rigidbody>()},
        {nameof<SphereCollider>(), GetComponentTypeId<SphereCollider>()},
        {nameof<AABBCollider>(), GetComponentTypeId<AABBCollider>()},
        {nameof<OBBCollider>(), GetComponentTypeId<OBBCollider>()},
        {nameof<CapsuleCollider>(), GetComponentTypeId<CapsuleCollider>()},
        {nameof<SegmentCollider>(), GetComponentTypeId<SegmentCollider>()},
        {nameof<RayCollider>(), GetComponentTypeId<RayCollider>()},
        {nameof<CollisionPushBackInfo>(), GetComponentTypeId<CollisionPushBackInfo>()},
    };

    std::unordered_set<uint32_t> seenIndices;
    bool allDistinct  = true;
    bool allWithinCap = true;

    for (const auto& [typeName, index] : indexMap) {
        const bool distinct  = seenIndices.insert(index).second;
        const bool withinCap = (index < static_cast<uint32_t>(kBitmaskCap));
        allDistinct &= distinct;
        allWithinCap &= withinCap;

        result.diagnosticLines.push_back(std::format(
            "[{}] {:<24} index={} distinct={} within64={}",
            (distinct && withinCap) ? "ok  " : "FAIL",
            typeName, index, distinct, withinCap));
    }

    const bool countMatches = (indexMap.size() == kExpectedTypeCount);
    result.diagnosticLines.push_back(std::format(
        "registered={} (expected {})  allDistinct={}  allWithinCap(<{})={}",
        indexMap.size(), kExpectedTypeCount, allDistinct, kBitmaskCap, allWithinCap));

    result.passed = allDistinct && allWithinCap && countMatches;

    return result;
}

/// <summary>
/// ケース4: X,Y,Zを登録 -> UnregisterComponentArray(nameof&lt;Y&gt;()) -> X と Z について判定式を確認する。
/// (FAIL またはクラッシュ期待。テストスイートの中で必ず最後に実行する)
///
/// X=Transform, Y=Rigidbody, Z=SphereCollider とし、ケース1と全く同じ順序で登録する。
/// ケース1がテストスイートの先頭で必ず実行されることで、この3型のstatic キャッシュは
/// 既に Transform=0, Rigidbody=1, SphereCollider=2 に確定している。このケースが新しく作る
/// repoもケース1と同じ順序で登録するため、Unregisterするまでは「たまたま一致している」の
/// ではなく「必然的に一致する」状態から出発できる(=ケース2/3のような登録順の違いに由来する
/// 不一致とこのケース固有の現象を混同しないための設計)。
///
/// UnregisterComponentArray(nameof&lt;Rigidbody&gt;()) は componentArrays_ の添字1を
/// erase する。ComponentRepository::UnregisterComponentArray の実装は
/// 「eraseで後続要素を詰める -> vectorのsizeを1減らす」ため、添字0(Transform)は無傷だが、
/// 添字2にあった SphereCollider は添字1へ詰められる。static キャッシュは
/// UnregisterComponentArray の存在を知らないため Transform=0, SphereCollider=2 のまま残り、
/// SphereCollider側の添字2は縮んだ後のcomponentArrays_(size=2)に対して範囲外になる。
/// std::vector::operator[]の範囲外アクセスは未定義動作であり、実装によっては誤ったポインタを
/// 黙って返すこともあれば、その場でプロセスが落ちることもある。どちらに転んでも
/// 「バグを検出できた」という意味では正しい結果である。
///
/// このケースを最後に置いているのは、上記の理由でプロセスごと落ちる可能性があり、
/// 後続のケースを巻き添えにしないため。呼び出し側(MakeComponentTypeIdTestCases)は
/// この関数を配列の最後に置くことでこれを保証する。
///
/// 【このケースだけ判定式(CheckMatchesOracle)に加えて独立の確認を行う理由】
/// 型ID化以前の UnregisterComponentArray は「削除した型のエントリを対応表から消すだけで、
/// それ以降にずれた型(ここではSphereCollider)の添字を書き直さない」実装だった。
/// 対応表の SphereCollider は erase 前の値(2)のまま残り続け、テンプレート版・オラクル版の
/// **両方**が同じ古い/範囲外の添字を参照する。両方が同じ壊れた値(実測ではnullptr)を返すため、
/// 「両者が一致するか」しか見ない判定式ではこの食い違いを検出できず、見かけ上PASSしてしまった。
/// これは static キャッシュ固有の寿命バグとは別に存在する、ComponentRepository側の
/// もう一段深いバグである(修正はユーザーの担当であり、ここでは行わない)。
/// このテストとしての責務は「バグを見逃さないこと」なので、Unregister前に生きていた
/// SphereColliderの実体へ、Unregister後もオラクル経由で辿り着けるかを別途確認する。
/// </summary>
TestCaseResult AfterUnregister_RemainingIndicesStayValid() {
    TestCaseResult result;

    ComponentRepository repo;
    RegisterSharedThreeTypes(repo); // X=Transform(0), Y=Rigidbody(1), Z=SphereCollider(2)

    // Unregister前に「生きているSphereCollider配列」の実体をオラクル経由で控えておく。
    // 下の判定式だけでは検出できない食い違いがあるため、この事前記録が必要になる
    // (理由は本関数のDoxygenコメント末尾を参照)。
    IComponentArray* sphereBeforeUnregister = repo.GetComponentArray(nameof<SphereCollider>());

    repo.UnregisterComponentArray(nameof<Rigidbody>());

    result.diagnosticLines.push_back("-- after UnregisterComponentArray(Rigidbody) --");
    CheckMatchesOracle<Transform>(repo, result); // 削除位置(添字1)より前 -> 無傷のはず
    CheckMatchesOracle<SphereCollider>(repo, result); // 削除位置より後 -> ここで崩れる想定(が、下記の理由で見逃しうる)

    // 判定式が偶然一致してPASSに見えてしまう場合を捕まえるための独立確認:
    // Unregister前に取得したSphereColliderの実体へ、Unregister後もオラクル経由で
    // 辿り着けるはず(添字が「位置」ではなく型IDなら、他の型の枠は動かないため)。
    // 辿り着けなければ、判定式の結果に関わらずこのケースをFAILとして扱う。
    IComponentArray* sphereAfterUnregister = repo.GetComponentArray(nameof<SphereCollider>());
    if (sphereBeforeUnregister != nullptr && sphereAfterUnregister == sphereBeforeUnregister) {
        result.diagnosticLines.push_back(std::format(
            "[ok]   SphereCollider reachable via oracle after Unregister: before={} after={}",
            PtrToString(sphereBeforeUnregister), PtrToString(sphereAfterUnregister)));
    } else {
        result.passed = false;
        result.diagnosticLines.push_back(std::format(
            "[FAIL] SphereCollider unreachable via oracle after Unregister(Rigidbody): "
            "before={} after={}  (the index of a surviving type moved when the erased element was "
            "removed, so both the template path AND the string-lookup oracle read the same stale slot)",
            PtrToString(sphereBeforeUnregister), PtrToString(sphereAfterUnregister)));
    }

    return result;
}

std::vector<TestCaseEntry> MakeComponentTypeIdTestCases() {
    // 実行順序は「1, 2, 3, 5, 4」。ケース番号どおりではないことに注意。
    // 理由は各ケース関数のDoxygenコメントおよびComponentTypeIdTests.hを参照。
    return {
        {"SameRepository_TemplateMatchesStringLookup", SameRepository_TemplateMatchesStringLookup},
        {"TwoRepositories_DifferentRegistrationOrder", TwoRepositories_DifferentRegistrationOrder},
        {"AfterClear_ReRegisterInDifferentOrder", AfterClear_ReRegisterInDifferentOrder},
        {"TypeIndicesAreDistinctAndWithinCap", TypeIndicesAreDistinctAndWithinCap},
        {"AfterUnregister_RemainingIndicesStayValid", AfterUnregister_RemainingIndicesStayValid},
    };
}

} // namespace OriGine::Test
