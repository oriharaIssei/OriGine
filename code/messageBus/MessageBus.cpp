#include "MessageBus.h"

// このファイルは GetInstance() 1つのためだけに存在する。MessageBus のそれ以外のメンバは
// 全部テンプレート(Subscribe/Emit/Unsubscribe/UnsubscribeAll)か、静的ローカルを持たない
// inline(Update)なので、境界を越えても実体が割れない(docs/plans/phase-04.md 4D 3)。
// GetInstance() だけは「関数ローカル static の実体をどのモジュールに1つ持たせるか」の
// 問題があるため、ここに出して OriGine.dll にだけ実体を置く。

MessageBus* MessageBus::GetInstance() {
    static MessageBus instance;
    return &instance;
}
