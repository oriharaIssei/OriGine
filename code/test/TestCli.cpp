#include "test/TestCli.h"

/// stl
#include <algorithm>
#include <cstring>
#include <iostream>

/// test
#include "test/TestRunner.h"
#include "test/TestSuiteRegistry.h"

/// logger
#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Test {

namespace {
constexpr const char* kTestFlag        = "--test";
constexpr const char* kTestSuitePrefix = "--test=";

// RunCliTestsIfRequested() の戻り値(bool)は既存の --bench と同じ「実行したか否か」の意味に
// 揃えたいが、テストは合否も呼び出し元へ伝える必要がある。シグネチャを変えずに両方満たすため、
// 合否だけをこの変数へ退避し GetLastTestExitCode() 経由で取り出せるようにしている。
int s_lastExitCode = 0;

bool StartsWith(const std::string& _s, const char* _prefix) {
    const size_t len = std::strlen(_prefix);
    return _s.size() >= len && _s.compare(0, len, _prefix) == 0;
}

/// <summary>
/// 未知のスイート名が指定されたときに、既知のスイート一覧をログ/標準出力へ書く。
/// </summary>
void PrintKnownSuites(const std::vector<TestSuite>& _suites) {
    std::string joined;
    for (size_t i = 0; i < _suites.size(); ++i) {
        if (i != 0) {
            joined += ", ";
        }
        joined += _suites[i].name;
    }
    LOG_ERROR("Known suites: {}", joined);
    std::cout << "Known suites: " << joined << std::endl;
}

} // namespace

bool RunCliTestsIfRequested(const std::vector<std::string>& _commandLines) {
    bool testRequested = false;
    bool suiteSpecified = false;
    std::string requestedSuite;

    for (const std::string& arg : _commandLines) {
        if (arg == kTestFlag) {
            testRequested = true;
        } else if (StartsWith(arg, kTestSuitePrefix)) {
            testRequested  = true;
            suiteSpecified = true;
            requestedSuite = arg.substr(std::strlen(kTestSuitePrefix));
        }
    }
    if (!testRequested) {
        return false;
    }

    const std::vector<TestSuite>& suites = GetAllTestSuites();

    if (suiteSpecified) {
        // 名前指定: 知らないスイート名は非0で終了させる(黙って全件実行にフォールバックしない)。
        auto it = std::find_if(suites.begin(), suites.end(),
            [&requestedSuite](const TestSuite& _suite) { return _suite.name == requestedSuite; });

        if (it == suites.end()) {
            LOG_ERROR("Unknown test suite: '{}'", requestedSuite);
            std::cout << "Unknown test suite: '" << requestedSuite << "'" << std::endl;
            PrintKnownSuites(suites);
            s_lastExitCode = 1;
            return true;
        }

        LOG_INFO("Test mode requested (--test={}). Running '{}' suite.", requestedSuite, it->name);
        std::cout << "Test mode requested (--test=" << requestedSuite << "). Running '" << it->name << "' suite." << std::endl;

        s_lastExitCode = RunTestCases(it->makeCases(), it->name);
        return true;
    }

    // スイート未指定: 登録順に全件実行し、1つでもFAILがあれば全体をFAIL扱いにする。
    LOG_INFO("Test mode requested (--test). Running all {} suite(s).", suites.size());
    std::cout << "Test mode requested (--test). Running all " << suites.size() << " suite(s)." << std::endl;

    int aggregateExitCode = 0;
    for (const TestSuite& suite : suites) {
        const int suiteExitCode = RunTestCases(suite.makeCases(), suite.name);
        if (suiteExitCode != 0) {
            aggregateExitCode = suiteExitCode;
        }
    }
    s_lastExitCode = aggregateExitCode;

    return true;
}

int GetLastTestExitCode() {
    return s_lastExitCode;
}

} // namespace OriGine::Test
