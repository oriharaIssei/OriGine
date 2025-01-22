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
#include "module/editor/IEditor.h"
//lib
#include "globalVariables/SerializedField.h"
//transform
#include "transform/CameraTransform.h"

class Effect;
/// <summary>
/// 編集時のみに使用するクラス
/// </summary>
class EffectEditor
    : public IModule {
public:
    EffectEditor();
    ~EffectEditor();

    void Init();
    void Update();
    void Draw();
    void Finalize();

private:
    void MenuBarUpdate();

private:
    std::unordered_map<std::string, std::unique_ptr<Effect>> effects_;
    Effect* currentEditEffect_ = nullptr;

    // 新しい Emitterを 作成する ための もの
    bool isOpenedCrateWindow_    = false;
    std::string newInstanceName_ = "NULL";
};
