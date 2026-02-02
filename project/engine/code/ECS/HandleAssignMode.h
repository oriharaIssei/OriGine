#pragma once

namespace OriGine {

/// <summary>
/// JSONからEntity/Componentを生成する際のHandle割り当て方法.
/// </summary>
enum class HandleAssignMode {
    /// <summary>
    /// JSONに保存されているHandleを使用する.
    /// Handleがない場合は新規生成.
    /// </summary>
    UseSaved,
    /// <summary>
    /// 常に新しいHandleを生成する.
    /// </summary>
    GenerateNew
};

} // namespace OriGine
