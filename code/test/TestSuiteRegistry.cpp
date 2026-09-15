#include "test/TestSuiteRegistry.h"

/// test
#include "test/ComponentTypeIdTests.h"
#include "test/LayoutTests.h"

namespace OriGine::Test {

const std::vector<TestSuite>& GetAllTestSuites() {
    // 登録順がそのまま "--test"(全件実行)時の実行順になる。
    // component-type-id は既存5ケース(未定義動作でプロセスごと落ちうるケースを含む)を
    // 保つために先頭に置く。layout は純粋な出力用スイートでクラッシュしないので後ろでよい。
    static const std::vector<TestSuite> suites = {
        {"component-type-id", &MakeComponentTypeIdTestCases},
        {"layout", &MakeLayoutTestCases},
    };
    return suites;
}

} // namespace OriGine::Test
