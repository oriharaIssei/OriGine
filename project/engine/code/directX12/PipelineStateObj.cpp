#include "PipelineStateObj.h"

void OriGine::PipelineStateObj::Finalize() {
    rootSignature.Reset();
    pipelineState.Reset();
}
