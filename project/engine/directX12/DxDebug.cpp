#include "DxDebug.h"

#include <dxgi1_6.h>
#include <dxgidebug.h>

DxDebug::DxDebug(){
#ifdef _DEBUG
    debugController_ = nullptr;
	//デバッグレイヤーをオンに
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		//デバッグレイヤーの有効化
        debugController_->EnableDebugLayer();
		//GPU側でもデバッグさせる
        debugController_->SetEnableGPUBasedValidation(TRUE);
	}
#endif // DEBUG 
}

DxDebug::~DxDebug(){
#ifdef _DEBUG
	IDXGIDebug1 *debug;
	if(SUCCEEDED(DXGIGetDebugInterface1(0,IID_PPV_ARGS(&debug)))){
		debug->ReportLiveObjects(DXGI_DEBUG_ALL,DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP,DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12,DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}
#endif // DEBUG 
}
