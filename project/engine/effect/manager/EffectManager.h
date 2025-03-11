#pragma once

///stl
//memory
#include <memory>
//container
#include <array>
#include <unordered_map>
#include <vector>
//string
#include <string>

///engine
//dx12Object
#include "directX12/DxCommand.h"
#include "directX12/DxSrvArray.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
//module
#include "module/IModule.h"
//lib
#include "globalVariables/SerializedField.h"
//transform
#include "component/transform/CameraTransform.h"

class Effect;
class Emitter;
class EffectManager
    : public IModule {
    friend class Effect;
    friend class Emitter;

public:
    static EffectManager* getInstance();
    void Init();
    void Finalize();
    void PreDraw();

    void Edit();
    void DrawDebug();

    void ChangeBlendMode(BlendMode mode);

private:
    EffectManager();
    ~EffectManager();
    EffectManager(const EffectManager&)                  = delete;
    const EffectManager& operator=(const EffectManager&) = delete;

    void CreatePso();

private:
    int32_t srvNum_ = 64;
    int32_t usingSrvNum_ = 0;
    std::shared_ptr<DxSrvArray> dxSrvArray_;

    std::unique_ptr<DxCommand> dxCommand_;

    std::array<std::string, kBlendNum> psoKey_;
    std::array<PipelineStateObj*, kBlendNum> pso_;
    BlendMode blendMode_ = BlendMode::Normal;

    std::unordered_map<std::string, std::unique_ptr<Effect>> effects_;

    bool emitterWindowedState_ = false;

    // 新しい Emitterを 作成する ための もの
    bool isOpenedCrateWindow_    = false;
    std::string newInstanceName_ = "NULL";
public:
    std::unique_ptr<Effect> CreateEffect(const std::string& name) ;
};
