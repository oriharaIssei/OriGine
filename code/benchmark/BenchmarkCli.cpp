#include "benchmark/BenchmarkCli.h"

/// stl
#include <charconv>
#include <cstring>

/// engine
#include "Engine.h"
#include "benchmark/BenchmarkCsv.h"
#include "benchmark/BenchmarkRunner.h"
#include "benchmark/BenchmarkSceneBuilder.h"
#include "benchmark/BenchmarkTypes.h"
#include "input/InputManager.h"
#include "scene/Scene.h"
#include "scene/SceneManager.h"

/// externals
#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Benchmark {

namespace {

constexpr const char* kBenchFlag      = "--bench";
constexpr const char* kEntitiesPrefix = "--bench-entities=";
constexpr const char* kExtentPrefix   = "--bench-extent=";
constexpr const char* kRadiusPrefix   = "--bench-radius=";
constexpr const char* kSeedPrefix     = "--bench-seed=";
constexpr const char* kFramesPrefix   = "--bench-frames=";
constexpr const char* kWarmupPrefix   = "--bench-warmup=";
constexpr const char* kCsvPrefix      = "--bench-csv=";

constexpr const char* kDefaultCsvPath = "./generated/benchmark/result.csv";

// ベンチ専用の一時シーン名。アプリ側resourceに同名のシーンJSONが存在しない限り、
// SceneFactory::BuildSceneByName は失敗ログを出すだけで安全に空のシーンを構築する。
constexpr const char* kBenchmarkSceneName = "__BenchmarkScene__";

bool StartsWith(const std::string& _s, const char* _prefix) {
    const size_t len = std::strlen(_prefix);
    return _s.size() >= len && _s.compare(0, len, _prefix) == 0;
}

/// <summary>整数/浮動小数を問わず std::from_chars で数値を解釈する(ロケール非依存)</summary>
template <typename T>
bool ParseNumber(const std::string& _text, T& _out) {
    if (_text.empty()) {
        return false;
    }
    const auto beginPtr = _text.data();
    const auto endPtr   = _text.data() + _text.size();
    const auto parsed    = std::from_chars(beginPtr, endPtr, _out);
    return parsed.ec == std::errc() && parsed.ptr == endPtr;
}

/// <summary>CLI引数を解析してBenchmarkConfig/CSVパスへ反映する</summary>
void ParseBenchmarkArgs(const std::vector<std::string>& _commandLines, BenchmarkConfig& _config, std::string& _csvPath) {
    for (const std::string& arg : _commandLines) {
        if (StartsWith(arg, kEntitiesPrefix)) {
            uint32_t v{};
            if (ParseNumber(arg.substr(std::strlen(kEntitiesPrefix)), v)) {
                _config.entityCount = v;
            }
        } else if (StartsWith(arg, kExtentPrefix)) {
            float v{};
            if (ParseNumber(arg.substr(std::strlen(kExtentPrefix)), v)) {
                _config.extent = v;
            }
        } else if (StartsWith(arg, kRadiusPrefix)) {
            float v{};
            if (ParseNumber(arg.substr(std::strlen(kRadiusPrefix)), v)) {
                _config.radius = v;
            }
        } else if (StartsWith(arg, kSeedPrefix)) {
            uint32_t v{};
            if (ParseNumber(arg.substr(std::strlen(kSeedPrefix)), v)) {
                _config.seed = v;
            }
        } else if (StartsWith(arg, kFramesPrefix)) {
            uint32_t v{};
            if (ParseNumber(arg.substr(std::strlen(kFramesPrefix)), v)) {
                _config.frames = v;
            }
        } else if (StartsWith(arg, kWarmupPrefix)) {
            uint32_t v{};
            if (ParseNumber(arg.substr(std::strlen(kWarmupPrefix)), v)) {
                _config.warmup = v;
            }
        } else if (StartsWith(arg, kCsvPrefix)) {
            _csvPath = arg.substr(std::strlen(kCsvPrefix));
        }
    }
}

} // namespace

bool RunCliBenchmarkIfRequested(const std::vector<std::string>& _commandLines) {
    bool benchRequested = false;
    for (const std::string& arg : _commandLines) {
        if (arg == kBenchFlag) {
            benchRequested = true;
            break;
        }
    }
    if (!benchRequested) {
        return false;
    }

    BenchmarkConfig config;
    std::string csvPath = kDefaultCsvPath;
    ParseBenchmarkArgs(_commandLines, config, csvPath);

    LOG_INFO("Benchmark mode requested: entities={} extent={} radius={} seed={} frames={} warmup={} csv={}",
        config.entityCount, config.extent, config.radius, config.seed, config.frames, config.warmup, csvPath);

    Engine* engine = Engine::GetInstance();

    // ベンチ専用のシーンを独立したSceneManagerで構築する(アプリ本来のシーンには一切触れない)
    SceneManager benchSceneManager;
    benchSceneManager.Initialize(
        kBenchmarkSceneName,
        InputManager::GetInstance()->GetKeyboard(),
        InputManager::GetInstance()->GetMouse(),
        InputManager::GetInstance()->GetGamePad());

    Scene* scene = benchSceneManager.GetCurrentScene();
    if (!scene) {
        LOG_ERROR("RunCliBenchmarkIfRequested: failed to create benchmark scene.");
        return true; // --bench が指定された以上、通常フローへは戻さずアプリを終了させる
    }

    BuildBenchmarkScene(scene, config);

    const BenchmarkResult result = RunBenchmarkLoop(engine, scene, config);

    if (!WriteBenchmarkCsv(csvPath, result.summary_, result.scopes_, result.frames_)) {
        LOG_ERROR("RunCliBenchmarkIfRequested: failed to write CSV to '{}'.", csvPath);
    }

    scene->Finalize();

    return true;
}

} // namespace OriGine::Benchmark
