#include "DxDebug.h"

/// api dxgi
#include <dxgi1_6.h>
#include <dxgidebug.h>

/// engine
#include "Engine.h"
#include "directX12/DxDevice.h"

/// lib
#include "lib/logger/Logger.h"

DxDebug::DxDebug() {
#ifdef _DEBUG
    InitializeDebugger();
#endif // DEBUG
}

DxDebug::~DxDebug() {
#ifdef _DEBUG
    FinalizeDebugger();
#endif // DEBUG
}

void DxDebug::InitializeDebugger() { // デバッグレイヤーをオンに
    if (debugController_) {
        LOG_CRITICAL("DebugController is already initialized.");
        return;
    }
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
        // デバッグレイヤーの有効化
        debugController_->EnableDebugLayer();
        // GPU側でもデバッグさせる
        debugController_->SetEnableGPUBasedValidation(TRUE);
    }
}

void DxDebug::FinalizeDebugger() {
    infoQueue_.Reset();
    IDXGIDebug1* debug;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
        debug->Release();
    }
}

void DxDebug::CreateInfoQueue() {
    if (debugController_) {
        HRESULT result =
            Engine::getInstance()->getDxDevice()->getDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue_));

        if (FAILED(result)) {
            LOG_ERROR("FAILED Create QuaerInterface. \n massage :" + result);
        }
    }
}

void DxDebug::setDebugMessageSeverity(D3D12_MESSAGE_SEVERITY severity) {
    if (infoQueue_) {
        infoQueue_->SetBreakOnSeverity(severity, TRUE);
    }
}
