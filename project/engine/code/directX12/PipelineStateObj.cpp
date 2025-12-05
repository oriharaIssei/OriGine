#include "PipelineStateObj.h"

using namespace OriGine;

void PipelineStateObj::Finalize() {
    rootSignature.Reset();
    pipelineState.Reset();
}
