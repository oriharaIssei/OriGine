#include "test/TestRunner.h"

/// stl
#include <iostream>

/// logger
#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Test {

int RunTestCases(const std::vector<TestCaseEntry>& _cases) {
    const std::string bar(60, '=');

    LOG_INFO("{}", bar);
    LOG_INFO("ComponentTypeId regression suite: {} case(s)", _cases.size());
    LOG_INFO("{}", bar);
    std::cout << bar << "\n";
    std::cout << "ComponentTypeId regression suite: " << _cases.size() << " case(s)\n";
    std::cout << bar << std::endl;

    uint32_t passedCount = 0;
    uint32_t failedCount = 0;

    for (const TestCaseEntry& entry : _cases) {
        // 実行前にケース名を出す。std::endl / LOG_INFO はいずれもこの時点で確実にflushされる
        // (cout側はstd::endlで明示flush、ログ側はLogger.cppのflush_on設定で毎回flushされる)。
        // これより後でプロセスが落ちても、「最後に RUN と出ていたケース」から落ちた場所を特定できる。
        LOG_INFO("[ RUN  ] {}", entry.name);
        std::cout << "[ RUN  ] " << entry.name << std::endl;

        TestCaseResult result = entry.func();

        // 診断行はPASS/FAILを問わず出す。各行の文面([ok]/[FAIL]プレフィックス)は
        // ComponentTypeIdTests.cpp 側で組み立てている(ここではログレベルを分けない)。
        for (const std::string& line : result.diagnosticLines) {
            LOG_INFO("         {}", line);
            std::cout << "         " << line << "\n";
        }

        if (result.passed) {
            ++passedCount;
            LOG_INFO("[ PASS ] {}", entry.name);
            std::cout << "[ PASS ] " << entry.name << std::endl;
        } else {
            ++failedCount;
            LOG_ERROR("[ FAIL ] {}", entry.name);
            std::cout << "[ FAIL ] " << entry.name << std::endl;
        }
    }

    LOG_INFO("{}", bar);
    LOG_INFO("{} passed, {} failed", passedCount, failedCount);
    LOG_INFO("{}", bar);
    std::cout << bar << "\n";
    std::cout << passedCount << " passed, " << failedCount << " failed\n";
    std::cout << bar << std::endl;

    return (failedCount == 0) ? 0 : 1;
}

} // namespace OriGine::Test
