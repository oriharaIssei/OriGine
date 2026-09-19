#include "benchmark/BenchmarkSerializeRunner.h"

/// stl
#include <chrono>

/// engine
#include "benchmark/BenchmarkSceneBuilder.h"
#include "input/InputManager.h"
#include "scene/Scene.h"
#include "scene/SceneFactory.h"
#include "scene/SceneManager.h"

/// profiler
#include "profiler/AllocationCounter.h"

/// externals
#include <nlohmann/json.hpp>

#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Benchmark {

namespace {

// ベンチ専用の一時シーン名。source/targetを分けているのは、SceneManagerが構築する
// FileWatcherの監視パス(kApplicationResourceDirectory + "/scene/" + 名前 + ".json")が
// シーンごとに独立するようにするため(実害は無い見込みだが、2つのSceneManagerが同じ
// パスを監視する状態を作らない)。どちらの名前にも対応するシーンJSONは存在しないため、
// BuildSceneByNameは失敗ログを出すだけで安全に空のシーンを構築する(BenchmarkCli.cppと同じ前提)。
constexpr const char* kSerializeSourceSceneName = "__BenchmarkSerializeSource__";
constexpr const char* kSerializeTargetSceneName = "__BenchmarkSerializeTarget__";

using Clock = std::chrono::steady_clock;

double ElapsedMs(Clock::time_point _begin, Clock::time_point _end) {
    return std::chrono::duration<double, std::milli>(_end - _begin).count();
}

} // namespace

std::vector<SerializeBenchRecord> RunSerializeBenchmark(const BenchmarkConfig& _config, uint32_t _repeat) {
    std::vector<SerializeBenchRecord> records;

    if (_repeat == 0) {
        LOG_ERROR("RunSerializeBenchmark: _repeat == 0.");
        return records;
    }
    records.reserve(static_cast<size_t>(_repeat) * 2);

    // ソースシーンは1つだけ構築する。CreateSceneJsonFromSceneは読み取り専用の経路なので、
    // 同じシーンから何度保存しても状態は変わらない(_repeat回とも同じ結果になるはず、という
    // 前提そのものを確かめるための計測)。
    SceneManager sourceManager;
    sourceManager.Initialize(
        kSerializeSourceSceneName,
        InputManager::GetInstance()->GetKeyboard(),
        InputManager::GetInstance()->GetMouse(),
        InputManager::GetInstance()->GetGamePad());

    Scene* sourceScene = sourceManager.GetCurrentScene();
    if (!sourceScene) {
        LOG_ERROR("RunSerializeBenchmark: failed to create source scene.");
        return records;
    }
    BuildBenchmarkScene(sourceScene, _config);

    SceneFactory factory;

    for (uint32_t repeat = 1; repeat <= _repeat; ++repeat) {
        // --- 保存 ---
        const AllocationCounter::CumulativeStats beforeSave = AllocationCounter::GetCumulativeStats();
        const Clock::time_point saveBegin                    = Clock::now();
        nlohmann::json sceneJson                              = factory.CreateSceneJsonFromScene(sourceScene);
        const Clock::time_point saveEnd                       = Clock::now();
        const AllocationCounter::CumulativeStats afterSave    = AllocationCounter::GetCumulativeStats();

        // JSONのバイト数は実ファイル書き出し(SceneJsonRegistry::SaveSceneのsetw(4))と
        // 同じインデント幅で数える。dumpは計測区間の外で行う
        // (「保存」= シーンをJSONオブジェクトへ変換する処理の時間・確保回数であって、
        // テキスト化のコストを混ぜないため)。
        const uint64_t jsonBytes = sceneJson.dump(4).size();

        SerializeBenchRecord saveRecord;
        saveRecord.repeat_      = repeat;
        saveRecord.case_        = "save";
        saveRecord.ms_          = ElapsedMs(saveBegin, saveEnd);
        saveRecord.allocCount_  = afterSave.allocCount_ - beforeSave.allocCount_;
        saveRecord.allocBytes_  = afterSave.allocBytes_ - beforeSave.allocBytes_;
        saveRecord.jsonBytes_   = jsonBytes;
        saveRecord.entityCount_ = _config.entityCount;
        records.push_back(saveRecord);

        // --- 読み込み ---
        // 毎回まっさらなシーンに読み込む。同じシーンへ繰り返し読み込むと2回目以降は
        // 既存Handleへの重複登録という1回目と違う経路を通ってしまい、決定性の検証にならない。
        SceneManager targetManager;
        targetManager.Initialize(
            kSerializeTargetSceneName,
            InputManager::GetInstance()->GetKeyboard(),
            InputManager::GetInstance()->GetMouse(),
            InputManager::GetInstance()->GetGamePad());

        Scene* targetScene = targetManager.GetCurrentScene();
        if (!targetScene) {
            LOG_ERROR("RunSerializeBenchmark: failed to create target scene (repeat {}).", repeat);
            break;
        }

        const AllocationCounter::CumulativeStats beforeLoad = AllocationCounter::GetCumulativeStats();
        const Clock::time_point loadBegin                    = Clock::now();
        factory.BuildSceneFromJson(targetScene, sceneJson);
        const Clock::time_point loadEnd                     = Clock::now();
        const AllocationCounter::CumulativeStats afterLoad   = AllocationCounter::GetCumulativeStats();

        SerializeBenchRecord loadRecord;
        loadRecord.repeat_      = repeat;
        loadRecord.case_        = "load";
        loadRecord.ms_          = ElapsedMs(loadBegin, loadEnd);
        loadRecord.allocCount_  = afterLoad.allocCount_ - beforeLoad.allocCount_;
        loadRecord.allocBytes_  = afterLoad.allocBytes_ - beforeLoad.allocBytes_;
        loadRecord.jsonBytes_   = jsonBytes;
        loadRecord.entityCount_ = _config.entityCount;
        records.push_back(loadRecord);

        targetScene->Finalize();
    }

    sourceScene->Finalize();

    LOG_INFO("RunSerializeBenchmark: {} record(s) collected ({} repeat(s), entities={}).",
        records.size(), _repeat, _config.entityCount);

    return records;
}

} // namespace OriGine::Benchmark
