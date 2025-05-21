#pragma once

#include <wrl.h>

#include <d3d12.h>

class DxDebug {
public:
    DxDebug();
    ~DxDebug();

    void InitializeDebugger();
    void FinalizeDebugger();

    void CreateInfoQueue();

private:
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_    = nullptr;

public:
    Microsoft::WRL::ComPtr<ID3D12Debug1> getDebugController() {
        return debugController_;
    }
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> getInfoQueue() {
        return infoQueue_;
    }

    void setDebugMessageSeverity(D3D12_MESSAGE_SEVERITY severity);
};
