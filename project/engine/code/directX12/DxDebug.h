#pragma once

#include <wrl.h>

#include <d3d12.h>

class DxDebug {
public:
    static DxDebug* getInstance() {
        static DxDebug instance;
        return &instance;
    }

    void InitializeDebugger();
    void FinalizeDebugger();

    void CreateInfoQueue();

private:
    DxDebug();
    ~DxDebug();
    DxDebug(const DxDebug&)            = delete;
    DxDebug& operator=(const DxDebug&) = delete;

private:
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_    = nullptr;

public:
    const Microsoft::WRL::ComPtr<ID3D12Debug1>& getDebugController() const {
        return debugController_;
    }
    Microsoft::WRL::ComPtr<ID3D12Debug1> getDebugControllerRef() {
        return debugController_;
    }
    const Microsoft::WRL::ComPtr<ID3D12InfoQueue>& getInfoQueue() const {
        return infoQueue_;
    }
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> getInfoQueueRef() {
        return infoQueue_;
    }

    void setDebugMessageSeverity(D3D12_MESSAGE_SEVERITY severity);
};
