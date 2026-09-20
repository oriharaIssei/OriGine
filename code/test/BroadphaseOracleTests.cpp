#include "test/BroadphaseOracleTests.h"

/// stl
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <iterator>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/// ECS
#include "entity/EntityHandle.h"
#include "entity/EntityRepository.h"
#include "system/collision/SpatialHash.h"

/// math
#include "Vector3.h"
#include "bounds/AABB.h"

/// benchmark(BenchScaleシナリオでentities/extent/radius/seedの既定値を揃えるためだけに使う)
#include "benchmark/BenchmarkTypes.h"

using namespace OriGine;

namespace OriGine::Test {

namespace {

/// <summary>
/// オラクル用の合成エンティティ1体。SpatialHashへ登録するAABBと、診断出力用のラベル・座標を
/// 1つにまとめたもの。実際のCollider/Transformコンポーネントは使わず、SpatialHashが受け取る
/// 「EntityHandle + AABB」の組だけを直接組み立てる(SpatialHash::Insert/GetAllPairsは
/// この2つだけで完結する公開APIのため)。
/// </summary>
struct SyntheticEntity {
    EntityHandle handle;
    std::string label;
    Vec3f center;
    float halfExtent = 0.0f; // 各軸共通の半径(球状Colliderのベンチ生成に合わせている)
};

/// <summary>
/// EntityRepositoryから新しいハンドルを1つ発行し、SyntheticEntityとして組み立てる。
/// EntityRepositoryはUUID発行とスロット管理だけを行いコンポーネントを一切持たないため、
/// このテストではコンポーネントを付けずハンドルだけを使う
/// (ecs-semanticsスイートと同じく、Scene/DirectX12デバイスは使わない)。
/// </summary>
SyntheticEntity MakeSyntheticEntity(EntityRepository& _repo, const std::string& _label, const Vec3f& _center, float _halfExtent) {
    SyntheticEntity e;
    e.handle     = _repo.CreateEntity(_label);
    e.label      = _label;
    e.center     = _center;
    e.halfExtent = _halfExtent;
    return e;
}

/// <summary>
/// 1軸ぶんのセル範囲(floor(min/cellSize) 〜 floor(max/cellSize))。
/// </summary>
struct CellRange1D {
    int32_t minCell;
    int32_t maxCell;
};

/// <summary>
/// SpatialHash::PositionToCell / GetCellRange とは一切コードを共有しない独立実装。
/// SpatialHash側は除算を避けるため事前計算した逆数(1.0f/cellSize_)を乗算しているが、
/// 乗算と除算は浮動小数点では必ずしも同じ丸め結果にならないため、ここではあえて素直な
/// 割り算を使う(「読んで明らかに正しい」ことを優先し、性能は一切考えない)。
/// </summary>
CellRange1D ComputeAxisCellRange(float _min, float _max, float _cellSize) {
    return CellRange1D{
        static_cast<int32_t>(std::floor(_min / _cellSize)),
        static_cast<int32_t>(std::floor(_max / _cellSize))};
}

/// <summary>1次元の整数区間[aMin,aMax]と[bMin,bMax]が重なっているか。</summary>
bool AxisRangesOverlap(const CellRange1D& _a, const CellRange1D& _b) {
    return _a.minCell <= _b.maxCell && _b.minCell <= _a.maxCell;
}

/// <summary>
/// 2体が1つ以上のセルを共有するかどうかを判定する。
/// 「x/y/z 全軸のセル範囲が重なっている」ことと「共通のセル(kx,ky,kz)が少なくとも1つ存在する」
/// ことは同値なので、実際にセルを1つずつ列挙しなくても3回の区間重なり判定で結論できる。
/// これがSpatialHash.cpp自身のコメントが明言する契約(「同じセルに入っているもの同士だけを
/// 候補として返す」)に対応するオラクルの定義であり、幾何学的な2つのAABBの重なり判定とは
/// 別物である点に注意(cellSizeが物体サイズよりずっと大きい設定では、同じセルに入っていても
/// 実際には接触していないペアが大量に生まれるのが正常であり、SpatialHashの役目は
/// あくまで「ナローフェーズにかける候補を絞り込むこと」でしかない)。
/// </summary>
bool SharesCellIndependent(const SyntheticEntity& _a, const SyntheticEntity& _b, float _cellSize) {
    const Vec3f aMin = _a.center - Vec3f(_a.halfExtent, _a.halfExtent, _a.halfExtent);
    const Vec3f aMax = _a.center + Vec3f(_a.halfExtent, _a.halfExtent, _a.halfExtent);
    const Vec3f bMin = _b.center - Vec3f(_b.halfExtent, _b.halfExtent, _b.halfExtent);
    const Vec3f bMax = _b.center + Vec3f(_b.halfExtent, _b.halfExtent, _b.halfExtent);

    if (!AxisRangesOverlap(ComputeAxisCellRange(aMin[X], aMax[X], _cellSize), ComputeAxisCellRange(bMin[X], bMax[X], _cellSize))) {
        return false;
    }
    if (!AxisRangesOverlap(ComputeAxisCellRange(aMin[Y], aMax[Y], _cellSize), ComputeAxisCellRange(bMin[Y], bMax[Y], _cellSize))) {
        return false;
    }
    return AxisRangesOverlap(ComputeAxisCellRange(aMin[Z], aMax[Z], _cellSize), ComputeAxisCellRange(bMin[Z], bMax[Z], _cellSize));
}

/// <summary>
/// O(N^2)の総当たりでオラクルのペア集合を作る。ペアは配列インデックスで正規化して
/// (常に小さいほうを先にして)返すため、EntityHandle同士の順序比較には一切依存しない。
/// 速度は考えない(重複排除にstd::setを使っているが、これはSpatialHash側の実装詳細である
/// 赤黒木ノードmallocの話とは無関係で、単に「集合として返す」ための素朴な入れ物として使っている)。
/// </summary>
std::set<std::pair<size_t, size_t>> ComputeOraclePairs(const std::vector<SyntheticEntity>& _entities, float _cellSize) {
    std::set<std::pair<size_t, size_t>> pairs;
    const size_t n = _entities.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (SharesCellIndependent(_entities[i], _entities[j], _cellSize)) {
                pairs.emplace(i, j);
            }
        }
    }
    return pairs;
}

/// <summary>
/// 2つのAABBが幾何学的に実際に重なっているかどうかを判定する(セルの概念は一切登場しない)。
/// x/y/z各軸で区間[min,max]が重なっていれば全体として重なっている、という標準的な
/// AABB-AABB重なり判定をそのまま書く(SpatialHashのどの関数も呼ばない独立実装)。
///
/// これは SharesCellIndependent とは判定基準そのものが別物である点が重要:
/// 「幾何学的に重なっているペアは、必ずセルを共有する」(重なった領域の1点が属するセルは、
/// floorが単調非減少関数である以上、両方のAABBが覆うセル範囲に必ず含まれるため)が、
/// 逆は成り立たない(セルを共有していても実際には重なっていないペアはいくらでもある)。
/// つまりこちらのほうが判定基準として厳しく、「これが候補集合から漏れたら実際の衝突を
/// 取りこぼす」という、内部実装(全破棄・全再構築でも差分更新でも階層グリッドでも)に
/// よらず常に成り立つべき最低限の安全性契約になる。
/// </summary>
bool GeometricallyOverlapsIndependent(const SyntheticEntity& _a, const SyntheticEntity& _b) {
    const Vec3f aMin = _a.center - Vec3f(_a.halfExtent, _a.halfExtent, _a.halfExtent);
    const Vec3f aMax = _a.center + Vec3f(_a.halfExtent, _a.halfExtent, _a.halfExtent);
    const Vec3f bMin = _b.center - Vec3f(_b.halfExtent, _b.halfExtent, _b.halfExtent);
    const Vec3f bMax = _b.center + Vec3f(_b.halfExtent, _b.halfExtent, _b.halfExtent);

    if (aMax[X] < bMin[X] || bMax[X] < aMin[X]) {
        return false;
    }
    if (aMax[Y] < bMin[Y] || bMax[Y] < aMin[Y]) {
        return false;
    }
    return !(aMax[Z] < bMin[Z] || bMax[Z] < aMin[Z]);
}

/// <summary>
/// O(N^2)の総当たりで「実際に幾何学的に重なっているペア」の集合を作る(セル共有オラクルの
/// ComputeOraclePairsとは別物。こちらは「絶対に見逃してはいけない集合」を作るためのもの)。
/// </summary>
std::set<std::pair<size_t, size_t>> ComputeGeometricOverlapPairs(const std::vector<SyntheticEntity>& _entities) {
    std::set<std::pair<size_t, size_t>> pairs;
    const size_t n = _entities.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (GeometricallyOverlapsIndependent(_entities[i], _entities[j])) {
                pairs.emplace(i, j);
            }
        }
    }
    return pairs;
}

/// <summary>
/// 実際のSpatialHash::GetAllPairsを呼んだ結果。unknownHandleCount_は、登録した覚えのない
/// EntityHandleを含むペアが返ってきた回数(本来0であるべき。0でなければテストを黙って
/// 通さず、それ自体を不具合として報告する)。
/// </summary>
struct SpatialHashOracleRun {
    std::set<std::pair<size_t, size_t>> pairs;
    size_t unknownHandleCount = 0;
};

/// <summary>
/// 実際のSpatialHashへ全エンティティを登録してGetAllPairsを呼び、結果をインデックスの
/// 正規化ペア集合へ変換する。ここで呼ぶのはSpatialHashの公開API(コンストラクタ/Insert/
/// GetAllPairs)だけで、セル分割やハッシュ、重複排除のロジックには一切踏み込まない。
/// </summary>
SpatialHashOracleRun ComputeSpatialHashPairs(const std::vector<SyntheticEntity>& _entities, float _cellSize) {
    std::unordered_map<EntityHandle, size_t> handleToIndex;
    handleToIndex.reserve(_entities.size());
    for (size_t i = 0; i < _entities.size(); ++i) {
        handleToIndex.emplace(_entities[i].handle, i);
    }

    SpatialHash hash(_cellSize);
    for (const SyntheticEntity& e : _entities) {
        Bounds::AABB aabb(e.center, Vec3f(e.halfExtent, e.halfExtent, e.halfExtent));
        hash.Insert(e.handle, aabb);
    }

    std::vector<std::pair<EntityHandle, EntityHandle>> rawPairs;
    hash.GetAllPairs(rawPairs);

    SpatialHashOracleRun run;
    for (const auto& [a, b] : rawPairs) {
        auto itA = handleToIndex.find(a);
        auto itB = handleToIndex.find(b);
        if (itA == handleToIndex.end() || itB == handleToIndex.end()) {
            // 登録した覚えのないハンドルが返ってきた。件数だけ数えて先へ進む
            // (計測基盤と同じ原則: 壊れたデータを黙って捨てずに必ず数えて報告する)。
            ++run.unknownHandleCount;
            continue;
        }
        size_t idxA = itA->second;
        size_t idxB = itB->second;
        if (idxA > idxB) {
            std::swap(idxA, idxB);
        }
        run.pairs.emplace(idxA, idxB);
    }
    return run;
}

/// <summary>エンティティ1体分の診断行を作る。</summary>
std::string DescribeEntity(const std::vector<SyntheticEntity>& _entities, size_t _index) {
    const SyntheticEntity& e = _entities[_index];
    return std::format("#{} '{}' center=({:.3f},{:.3f},{:.3f}) halfExtent={:.3f}",
        _index, e.label, e.center[X], e.center[Y], e.center[Z], e.halfExtent);
}

/// <summary>
/// オラクルとSpatialHashのペア集合を比較し、TestCaseResultへ合否と診断行を書き込む。
/// 一致しない場合は「オラクルにしかない(SpatialHashが取りこぼした)」
/// 「SpatialHashにしかない(オラクルが認めていない余剰)」の両方を、実体の座標つきで
/// 先頭 _maxSamples 件まで具体的に出す(全件出すと件数が多いときに読めないログになるため)。
/// </summary>
void CompareAndReport(
    TestCaseResult& _result,
    const std::string& _scenarioLabel,
    const std::vector<SyntheticEntity>& _entities,
    const std::set<std::pair<size_t, size_t>>& _oraclePairs,
    const SpatialHashOracleRun& _actual,
    size_t _maxSamples = 8) {

    std::vector<std::pair<size_t, size_t>> oracleOnly;
    std::set_difference(_oraclePairs.begin(), _oraclePairs.end(), _actual.pairs.begin(), _actual.pairs.end(), std::back_inserter(oracleOnly));

    std::vector<std::pair<size_t, size_t>> actualOnly;
    std::set_difference(_actual.pairs.begin(), _actual.pairs.end(), _oraclePairs.begin(), _oraclePairs.end(), std::back_inserter(actualOnly));

    const bool handlesOk = (_actual.unknownHandleCount == 0);
    const bool setsMatch  = oracleOnly.empty() && actualOnly.empty();
    const bool ok         = handlesOk && setsMatch;
    _result.passed &= ok;

    _result.diagnosticLines.push_back(std::format(
        "[{}] {}: entities={} oraclePairs={} spatialHashPairs={} unknownHandles={}",
        ok ? "ok  " : "FAIL", _scenarioLabel, _entities.size(), _oraclePairs.size(), _actual.pairs.size(), _actual.unknownHandleCount));

    if (!handlesOk) {
        _result.diagnosticLines.push_back(std::format(
            "  SpatialHash::GetAllPairsが登録していないEntityHandleを含むペアを{}組返した(対応関係が取れない)",
            _actual.unknownHandleCount));
    }

    if (!oracleOnly.empty()) {
        _result.diagnosticLines.push_back(std::format(
            "  オラクルにしかないペア(SpatialHashが取りこぼした真の候補): {}件", oracleOnly.size()));
        size_t shown = 0;
        for (const auto& [i, j] : oracleOnly) {
            if (shown >= _maxSamples) {
                _result.diagnosticLines.push_back(std::format("    ...他 {} 件省略", oracleOnly.size() - shown));
                break;
            }
            _result.diagnosticLines.push_back(std::format("    {}  <->  {}", DescribeEntity(_entities, i), DescribeEntity(_entities, j)));
            ++shown;
        }
    }

    if (!actualOnly.empty()) {
        _result.diagnosticLines.push_back(std::format(
            "  SpatialHashにしかないペア(オラクルが候補と認めていない余剰): {}件", actualOnly.size()));
        size_t shown = 0;
        for (const auto& [i, j] : actualOnly) {
            if (shown >= _maxSamples) {
                _result.diagnosticLines.push_back(std::format("    ...他 {} 件省略", actualOnly.size() - shown));
                break;
            }
            _result.diagnosticLines.push_back(std::format("    {}  <->  {}", DescribeEntity(_entities, i), DescribeEntity(_entities, j)));
            ++shown;
        }
    }
}

/// <summary>1シナリオぶんの比較を実行する共通ドライバ。</summary>
TestCaseResult RunOracleComparison(const std::string& _scenarioLabel, const std::vector<SyntheticEntity>& _entities, float _cellSize) {
    TestCaseResult result;
    result.passed = true;

    const std::set<std::pair<size_t, size_t>> oraclePairs = ComputeOraclePairs(_entities, _cellSize);
    const SpatialHashOracleRun actual                      = ComputeSpatialHashPairs(_entities, _cellSize);

    CompareAndReport(result, _scenarioLabel, _entities, oraclePairs, actual);
    return result;
}

/// <summary>
/// 「幾何学的に重なっているペアは、SpatialHashの候補集合に必ず含まれていなければならない」
/// という包含契約を検査する。セル共有オラクル(CompareAndReport)とは別の契約であり、
/// あえて別関数として分けている: SpatialHash側に「セルは共有しているが実際には重なっていない」
/// 余分な候補が混ざるのは正常(ブロードフェーズが候補を広げに絞り込むのは仕様どおり)なので
/// 余剰側は一切見ず、見逃し(false negative)だけを検出する。
/// </summary>
void CompareSubsetAndReport(
    TestCaseResult& _result,
    const std::string& _scenarioLabel,
    const std::vector<SyntheticEntity>& _entities,
    const std::set<std::pair<size_t, size_t>>& _geometricPairs,
    const SpatialHashOracleRun& _actual,
    size_t _maxSamples = 8) {

    std::vector<std::pair<size_t, size_t>> missed;
    std::set_difference(_geometricPairs.begin(), _geometricPairs.end(), _actual.pairs.begin(), _actual.pairs.end(), std::back_inserter(missed));

    const bool handlesOk = (_actual.unknownHandleCount == 0);
    const bool noMisses  = missed.empty();
    const bool ok        = handlesOk && noMisses;
    _result.passed &= ok;

    _result.diagnosticLines.push_back(std::format(
        "[{}] {}: entities={} geometricOverlapPairs={} spatialHashPairs={} unknownHandles={}",
        ok ? "ok  " : "FAIL", _scenarioLabel, _entities.size(), _geometricPairs.size(), _actual.pairs.size(), _actual.unknownHandleCount));

    if (!handlesOk) {
        _result.diagnosticLines.push_back(std::format(
            "  SpatialHash::GetAllPairsが登録していないEntityHandleを含むペアを{}組返した(対応関係が取れない)",
            _actual.unknownHandleCount));
    }

    if (!missed.empty()) {
        _result.diagnosticLines.push_back(std::format(
            "  見逃されたペア(幾何学的に接触しているのにSpatialHashの候補集合に無い): {}件", missed.size()));
        size_t shown = 0;
        for (const auto& [i, j] : missed) {
            if (shown >= _maxSamples) {
                _result.diagnosticLines.push_back(std::format("    ...他 {} 件省略", missed.size() - shown));
                break;
            }
            _result.diagnosticLines.push_back(std::format("    {}  <->  {}", DescribeEntity(_entities, i), DescribeEntity(_entities, j)));
            ++shown;
        }
    }
}

/// <summary>
/// 1シナリオぶんの包含契約チェックを実行する共通ドライバ。RunOracleComparisonと違い、
/// SpatialHash側の余剰候補は見ない(見逃しの有無だけを判定する)。
/// </summary>
TestCaseResult RunSubsetInclusionCheck(const std::string& _scenarioLabel, const std::vector<SyntheticEntity>& _entities, float _cellSize) {
    TestCaseResult result;
    result.passed = true;

    const std::set<std::pair<size_t, size_t>> geometricPairs = ComputeGeometricOverlapPairs(_entities);
    const SpatialHashOracleRun actual                         = ComputeSpatialHashPairs(_entities, _cellSize);

    CompareSubsetAndReport(result, _scenarioLabel, _entities, geometricPairs, actual);

    // 陽性対照: この契約チェックは「実際に接触しているペアが1組も無い」場合は何も検証していない
    // ことになり(空虚な成功)、意味を持たない。シナリオ側が意図どおり接触を生んでいるかを
    // ここで別途確認する(具体的な期待件数はシナリオごとに違うので、0件でないことだけ見る)。
    const bool hasAtLeastOneContact = !geometricPairs.empty();
    result.passed &= hasAtLeastOneContact;
    if (!hasAtLeastOneContact) {
        result.diagnosticLines.push_back(
            "  [FAIL] 配置に幾何学的な接触が1組も無い(この契約チェックが陽性対照になっていない。座標を見直すこと)");
    }

    return result;
}

} // namespace

/// <summary>
/// ケース1: エンティティが0体のとき、オラクル・SpatialHashともに空集合を返すこと。
/// GetAllPairsが空のcells_に対してクラッシュしたり不定のゴミを返したりしないかの回帰。
/// </summary>
TestCaseResult BroadphaseOracle_Empty() {
    EntityRepository repo;
    repo.Initialize();
    std::vector<SyntheticEntity> entities; // 意図的に0体
    return RunOracleComparison("Empty_ZeroEntities", entities, 10.0f);
}

/// <summary>
/// ケース2: エンティティが1体のときペアは常に0件であること(自分自身とはペアを作らない)。
/// </summary>
TestCaseResult BroadphaseOracle_Single() {
    EntityRepository repo;
    repo.Initialize();
    std::vector<SyntheticEntity> entities;
    entities.push_back(MakeSyntheticEntity(repo, "solo", Vec3f(3.0f, -7.0f, 42.0f), 1.0f));
    return RunOracleComparison("Single_OneEntity", entities, 10.0f);
}

/// <summary>
/// ケース3: まばらな配置(セルをまたがない)。どの2体もセルはおろか隣接セルにも入らないほど
/// 離して置くことで、「近くに何も無ければ0ペア」を保証する陰性対照(negative control)にする。
/// </summary>
TestCaseResult BroadphaseOracle_Sparse() {
    EntityRepository repo;
    repo.Initialize();
    std::vector<SyntheticEntity> entities;
    const float cellSize = 10.0f;
    // 半径1に対してセルサイズ10・配置間隔50(=5セル分)を取ることで、隣接セルにすら
    // 重ならない間隔を確保する。
    for (int i = 0; i < 12; ++i) {
        const float coord = static_cast<float>(i) * 50.0f;
        entities.push_back(MakeSyntheticEntity(repo, std::format("sparse#{}", i), Vec3f(coord, coord, coord), 1.0f));
    }
    return RunOracleComparison("Sparse_NoCellCrossing", entities, cellSize);
}

/// <summary>
/// ケース4: 密な配置(1セルに多数が入る)。全員を1セルの中心付近・セルサイズよりずっと小さい
/// 半径でまとめることで、全ペア(n*(n-1)/2)が候補になるはずの陽性対照(positive control)にする。
/// SpatialHashとの一致だけでなく、オラクル自身が期待どおりの件数を出しているかも別途確認する
/// (オラクルが常に空集合を返すような壊れ方をしていたら、SpatialHashとの一致比較自体が
/// 意味を失うため)。
/// </summary>
TestCaseResult BroadphaseOracle_Dense() {
    EntityRepository repo;
    repo.Initialize();
    std::vector<SyntheticEntity> entities;
    const float cellSize = 50.0f;
    std::mt19937 rng(777);
    std::uniform_real_distribution<float> jitter(-0.4f, 0.4f);
    constexpr int kCount = 24;
    for (int i = 0; i < kCount; ++i) {
        Vec3f center(25.0f + jitter(rng), 25.0f + jitter(rng), 25.0f + jitter(rng));
        entities.push_back(MakeSyntheticEntity(repo, std::format("dense#{}", i), center, 0.05f));
    }

    TestCaseResult result = RunOracleComparison("Dense_SingleCellManyEntities", entities, cellSize);

    const size_t expectedPairCount = static_cast<size_t>(kCount) * (kCount - 1) / 2;
    const size_t oraclePairCount   = ComputeOraclePairs(entities, cellSize).size();
    const bool selfCheckOk         = (oraclePairCount == expectedPairCount);
    result.passed &= selfCheckOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] Dense self-check: oracle produced {} pairs (expected all-pairs {} = {}*{}/2)",
        selfCheckOk ? "ok  " : "FAIL", oraclePairCount, expectedPairCount, kCount, kCount - 1));

    return result;
}

/// <summary>
/// ケース5: 全部が同一座標に重なる縮退ケース。半径0(点)のAABBが全員同じ座標に集まる状況
/// (スケール0のTransformなどで実際に起こりうる)。全ペアが候補になるはずの陽性対照。
/// </summary>
TestCaseResult BroadphaseOracle_Degenerate() {
    EntityRepository repo;
    repo.Initialize();
    std::vector<SyntheticEntity> entities;
    const float cellSize = 10.0f;
    constexpr int kCount = 15;
    for (int i = 0; i < kCount; ++i) {
        entities.push_back(MakeSyntheticEntity(repo, std::format("degenerate#{}", i), Vec3f(5.0f, 5.0f, 5.0f), 0.0f));
    }

    TestCaseResult result = RunOracleComparison("Degenerate_AllSamePoint", entities, cellSize);

    const size_t expectedPairCount = static_cast<size_t>(kCount) * (kCount - 1) / 2;
    const size_t oraclePairCount   = ComputeOraclePairs(entities, cellSize).size();
    const bool selfCheckOk         = (oraclePairCount == expectedPairCount);
    result.passed &= selfCheckOk;
    result.diagnosticLines.push_back(std::format(
        "[{}] Degenerate self-check: oracle produced {} pairs (expected all-pairs {})",
        selfCheckOk ? "ok  " : "FAIL", oraclePairCount, expectedPairCount));

    return result;
}

/// <summary>
/// ケース6: セル境界をまたぐ配置(空間ハッシュのバグの定番)。
/// cellSizeに3.0のような割り切れない値を使うのは意図的: SpatialHashは除算の代わりに
/// 事前計算した逆数(1.0f/cellSize_)を乗算する実装になっており、1.0f/3.0fは浮動小数点で
/// 正確には表現できない。セル境界ちょうど(3の倍数)に置いたエンティティで、割り算方式
/// (オラクル)と逆数乗算方式(SpatialHash)の丸めが食い違う余地がないかを突く。
/// 加えて、原点(x=0)をまたぐ配置と負の座標側の境界も混ぜ、floorではなくintキャストで
/// 切り捨てるバグ(負の値で0方向に切り捨てられ、原点をまたぐセルだけ2倍の幅になる)が
/// 再発していないかも同時に確認する。
/// </summary>
TestCaseResult BroadphaseOracle_BoundaryStraddle() {
    EntityRepository repo;
    repo.Initialize();
    std::vector<SyntheticEntity> entities;
    const float cellSize = 3.0f;

    entities.push_back(MakeSyntheticEntity(repo, "origin_straddle", Vec3f(0.0f, 0.0f, 0.0f), 0.01f)); // x=0(負セルとの境界)をまたぐ
    entities.push_back(MakeSyntheticEntity(repo, "pos_boundary_lo", Vec3f(2.99f, 0.0f, 0.0f), 0.02f)); // cell0/1境界(x=3)をまたぐ
    entities.push_back(MakeSyntheticEntity(repo, "pos_boundary_hi", Vec3f(3.01f, 0.0f, 0.0f), 0.001f)); // cell1側にわずかに入るだけ
    entities.push_back(MakeSyntheticEntity(repo, "neg_multiple", Vec3f(-3.0f, 0.0f, 0.0f), 0.01f)); // 負側のセルサイズ倍数ちょうど
    entities.push_back(MakeSyntheticEntity(repo, "pos_multiple", Vec3f(6.0f, 0.0f, 0.0f), 0.01f)); // 正側のセルサイズ倍数ちょうど
    entities.push_back(MakeSyntheticEntity(repo, "y_straddle_a", Vec3f(100.0f, 2.98f, 0.0f), 0.02f)); // x方向は孤立させ、y境界だけをまたぐ
    entities.push_back(MakeSyntheticEntity(repo, "y_straddle_b", Vec3f(100.0f, -0.01f, 0.0f), 0.02f)); // y_straddle_aとy=0セルを共有するはず
    entities.push_back(MakeSyntheticEntity(repo, "z_isolated", Vec3f(200.0f, 200.0f, 9.0f), 0.01f)); // どこともセルを共有しない孤立点

    return RunOracleComparison("BoundaryStraddle_CellEdges", entities, cellSize);
}

/// <summary>
/// ケース7: ベンチと同じ条件(entities 2000 / extent 500 / radius 1 / seed 12345 =
/// bench.ps1の既定値およびBenchmarkConfigの既定値)。BuildBenchmarkSceneはScene
/// (DirectX12デバイス初期化済みが前提)を経由しないと呼べず、他の回帰スイートと同じく
/// 本スイートもScene/DirectX12デバイスに依存しない方針にしているため、
/// BuildBenchmarkSceneと全く同じ乱数の呼び出し順序(位置3回 + 速度3回 per entity、
/// std::mt19937を同じseedで使用)をここでもなぞることで、同じ配置列を独立に再現する
/// (速度の値自体はSpatialHashに使わないので捨てるが、rngの消費回数を合わせないと
/// 以降のエンティティの位置がBuildBenchmarkSceneとずれてしまう)。
/// 2000体の総当たり(約200万組)にかかる実測時間はtest.ps1の実行時間出力を参照。
/// </summary>
TestCaseResult BroadphaseOracle_BenchScale() {
    EntityRepository repo;
    repo.Initialize();

    Benchmark::BenchmarkConfig config; // extent(500)/radius(1)/seedは既定値のまま使う
    config.entityCount = 2000; // bench.ps1の既定値(Entities=2000)に合わせる(BenchmarkConfig既定は10000)

    std::mt19937 rng(config.seed);
    const float halfExtent = config.extent * 0.5f;
    std::uniform_real_distribution<float> posDist(-halfExtent, halfExtent);
    // BenchmarkSceneBuilder.cppのkMaxInitialSpeedと同じ値。乱数消費順をそちらに揃えるためだけに使い、値自体は捨てる
    constexpr float kMaxInitialSpeed = 2.0f;
    std::uniform_real_distribution<float> velDist(-kMaxInitialSpeed, kMaxInitialSpeed);

    std::vector<SyntheticEntity> entities;
    entities.reserve(config.entityCount);
    for (uint32_t i = 0; i < config.entityCount; ++i) {
        Vec3f center(posDist(rng), posDist(rng), posDist(rng));
        // 位置のみ使用。BuildBenchmarkSceneの乱数消費順(速度3回)に合わせるための空読み
        // (uniform_real_distribution::operator()は[[nodiscard]]なので戻り値をvoidへ明示的に捨てる)
        static_cast<void>(velDist(rng));
        static_cast<void>(velDist(rng));
        static_cast<void>(velDist(rng));
        entities.push_back(MakeSyntheticEntity(repo, std::format("bench#{}", i), center, config.radius));
    }

    // SpatialHashのコンストラクタ既定値(100.0f)をそのまま使う。CollisionCheckSystemは
    // GlobalVariables経由で別の値に上書きされうるが、このテストはSpatialHash単体の契約を
    // 確認するのが目的であり、実行時設定の値には依存しない。
    return RunOracleComparison("BenchScale_2000Entities", entities, 100.0f);
}

/// <summary>
/// ケース8: 「幾何学的に重なっているペアは、SpatialHashの候補集合に必ず含まれていなければ
/// ならない」という包含契約を検査する。ケース1〜7(セル共有オラクル)とは別の契約であり、
/// SpatialHashの内部設計が全破棄・全再構築から差分更新へ変わっても(Phase 6でユーザーが
/// 行う予定の変更そのもの)成り立ち続けるべき最低限の安全網になる
/// (差分更新は「更新漏れで本来重なっているペアが候補から落ちる」事故が定番のため)。
///
/// 接触がほとんど無いcellSizeに対して半径が小さい配置(ケース1〜7の一部)では、この契約は
/// ほぼ何も検証しない空虚なチェックになってしまう。そのためここでは意図的に:
///  - 1体のAABBが複数セルにまたがり、かつ実際に隣のセルにいる相手と接触する配置
///    (spanning_big <-> neighbor_edge: 半径5と半径2、cellSize2に対してどちらも複数セルにまたがる)
///  - 同じ構図をもっと小さいスケールでも1組(boundary_pair_a <-> boundary_pair_b:
///    片方がセル境界をまたぎ、もう片方は隣のセルの中に収まっている)
///  - 別軸(y軸)でも同じ構図を1組(y_axis_spanning <-> y_axis_neighbor)、
///    x軸側の配置と絶対に混ざらない位置に置いて独立に確認する
/// という3系統の「実際に接触するペア」を用意し、いずれもSpatialHashの候補集合から
/// 取りこぼされていないかを見る。
/// </summary>
TestCaseResult BroadphaseOracle_GeometricOverlapMustBeCandidate() {
    EntityRepository repo;
    repo.Initialize();
    std::vector<SyntheticEntity> entities;
    const float cellSize = 2.0f;

    // 系統1: 半径がcellSizeよりずっと大きく、どちらも複数セルにまたがったうえで接触する。
    // spanning_big: x=[-5,5] (cellSize2で cell -3..2 の6セルにまたがる)
    // neighbor_edge: x=[4,8] (cell 2..4 の3セルにまたがる)。4<=5 なので x=[4,5] の範囲で実際に重なる。
    entities.push_back(MakeSyntheticEntity(repo, "spanning_big", Vec3f(0.0f, 0.0f, 0.0f), 5.0f));
    entities.push_back(MakeSyntheticEntity(repo, "neighbor_edge", Vec3f(6.0f, 0.0f, 0.0f), 2.0f));

    // 系統2: 小さいスケール版。boundary_pair_aはセル境界(x=20)をまたぎ、
    // boundary_pair_bは隣のセルの中に収まっている。それでも0.1単位だけ実際に重なる。
    entities.push_back(MakeSyntheticEntity(repo, "boundary_pair_a", Vec3f(20.0f, 0.0f, 0.0f), 0.3f)); // x=[19.7,20.3]
    entities.push_back(MakeSyntheticEntity(repo, "boundary_pair_b", Vec3f(20.5f, 0.0f, 0.0f), 0.3f)); // x=[20.2,20.8]

    // 系統3: y軸方向で同じ構図をもう1組。x軸側の2組とは座標が大きく離れているため干渉しない。
    entities.push_back(MakeSyntheticEntity(repo, "y_axis_spanning", Vec3f(0.0f, 20.0f, 0.0f), 1.5f)); // y=[18.5,21.5]
    entities.push_back(MakeSyntheticEntity(repo, "y_axis_neighbor", Vec3f(0.0f, 23.0f, 0.0f), 1.6f)); // y=[21.4,24.6]

    return RunSubsetInclusionCheck("GeometricOverlapMustBeCandidate", entities, cellSize);
}

std::vector<TestCaseEntry> MakeBroadphaseOracleTestCases() {
    return {
        {"BroadphaseOracle_Empty", BroadphaseOracle_Empty},
        {"BroadphaseOracle_Single", BroadphaseOracle_Single},
        {"BroadphaseOracle_Sparse", BroadphaseOracle_Sparse},
        {"BroadphaseOracle_Dense", BroadphaseOracle_Dense},
        {"BroadphaseOracle_Degenerate", BroadphaseOracle_Degenerate},
        {"BroadphaseOracle_BoundaryStraddle", BroadphaseOracle_BoundaryStraddle},
        {"BroadphaseOracle_BenchScale", BroadphaseOracle_BenchScale},
        {"BroadphaseOracle_GeometricOverlapMustBeCandidate", BroadphaseOracle_GeometricOverlapMustBeCandidate},
    };
}

} // namespace OriGine::Test
