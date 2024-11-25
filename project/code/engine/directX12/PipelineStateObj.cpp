#include "PipelineStateObj.h"

void PipelineStateObj::Finalize() {
	rootSignature.Reset();
	pipelineState.Reset();
}