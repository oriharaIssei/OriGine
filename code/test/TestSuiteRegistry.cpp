#include "test/TestSuiteRegistry.h"

/// test
#include "test/ComponentTypeIdTests.h"
#include "test/DescriptorTests.h"
#include "test/LayoutTests.h"

namespace OriGine::Test {

const std::vector<TestSuite>& GetAllTestSuites() {
    // 登録順がそのまま "--test"(全件実行)時の実行順になる。
    // component-type-id は既存5ケース(未定義動作でプロセスごと落ちうるケースを含む)を
    // 保つために先頭に置く。layout / descriptor は純粋な出力用スイートでクラッシュしないので
    // 後ろでよい。descriptor は Phase 3C(型ディスクリプタのコード生成)の確認用。
    static const std::vector<TestSuite> suites = {
        {"component-type-id", &MakeComponentTypeIdTestCases},
        {"layout", &MakeLayoutTestCases},
        {"descriptor", &MakeDescriptorTestCases},
    };
    return suites;
}

} // namespace OriGine::Test
