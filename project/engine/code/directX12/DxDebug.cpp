#include "DxDebug.h"

/// api dxgi
#include <dxgi1_6.h>
#include <dxgidebug.h>

/// stl
#include <comdef.h>
#include <format>

/// engine
#include "directX12/DxDevice.h"
#include "Engine.h"

/// lib
#include "lib/logger/Logger.h"

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
        // debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
        debug->Release();
    }
}

void DxDebug::CreateInfoQueue() {
    if (debugController_) {
        const Microsoft::WRL::ComPtr<ID3D12Device>& device = Engine::getInstance()->getDxDevice()->getDevice();
        HRESULT result                                     = device->QueryInterface(IID_PPV_ARGS(infoQueue_.GetAddressOf()));

        if (FAILED(result)) {
            _com_error err(result);
            std::wstring msg = std::format(
                L"ID3D12InfoQueue is not available. This is expected if the debug layer is not enabled. HRESULT: 0x{0:08X}, message: {1}",
                static_cast<unsigned int>(result), err.ErrorMessage());
            LOG_WARN(msg.c_str());
            infoQueue_.Reset();
        } else {
            LOG_INFO("ID3D12InfoQueue is available.");
            // デバッグメッセージの出力を有効にする
            infoQueue_->SetMuteDebugOutput(FALSE);
            // デバッグメッセージの詳細度を設定
            infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        }
    }
}

DxDebug::DxDebug() {}

DxDebug::~DxDebug() {
    FinalizeDebugger();
}

void DxDebug::setDebugMessageSeverity(D3D12_MESSAGE_SEVERITY severity) {
    if (infoQueue_) {
        infoQueue_->SetBreakOnSeverity(severity, TRUE);
    }
}
