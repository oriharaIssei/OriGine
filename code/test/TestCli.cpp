#include "test/TestCli.h"

/// stl
#include <iostream>

/// test
#include "test/ComponentTypeIdTests.h"
#include "test/TestRunner.h"

/// logger
#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Test {

namespace {
constexpr const char* kTestFlag = "--test";

// RunCliTestsIfRequested() の戻り値(bool)は既存の --bench と同じ「実行したか否か」の意味に
// 揃えたいが、テストは合否も呼び出し元へ伝える必要がある。シグネチャを変えずに両方満たすため、
// 合否だけをこの変数へ退避し GetLastTestExitCode() 経由で取り出せるようにしている。
int s_lastExitCode = 0;
} // namespace

bool RunCliTestsIfRequested(const std::vector<std::string>& _commandLines) {
    bool testRequested = false;
    for (const std::string& arg : _commandLines) {
        if (arg == kTestFlag) {
            testRequested = true;
            break;
        }
    }
    if (!testRequested) {
        return false;
    }

    LOG_INFO("Test mode requested (--test). Running ComponentTypeId regression suite.");
    std::cout << "Test mode requested (--test). Running ComponentTypeId regression suite." << std::endl;

    const std::vector<TestCaseEntry> cases = MakeComponentTypeIdTestCases();
    s_lastExitCode                         = RunTestCases(cases);

    return true;
}

int GetLastTestExitCode() {
    return s_lastExitCode;
}

} // namespace OriGine::Test
