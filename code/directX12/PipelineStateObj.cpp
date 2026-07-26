#include "PipelineStateObj.h"

/// <summary>
/// ルートシグネチャとパイプラインステートを解放する。
/// </summary>
void OriGine::PipelineStateObj::Finalize() {
    rootSignature.Reset();
    pipelineState.Reset();
}
