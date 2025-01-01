#pragma once

///stl
//memory
#include <functional>
#include <memory>
//container
#include <array>
#include <unordered_map>
//string
#include <string>

//directX12Object
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

///lib
#include "Thread/Thread.h"

///math
#include "Matrix4x4.h"

class AnimationObject3d;
class Object3d;
struct Model;
struct ModelNode;
struct ModelMeshData;
struct Material3D;
struct Animation;

class ModelManager {
    friend class AnimationObject3d;

    friend class Object3d;

public:
    static ModelManager* getInstance();

    std::unique_ptr<Model> Create(
        const std::string& directoryPath,
        const std::string& filename,
        std::function<void(Model* )> callBack = nullptr);

    void Init();
    void Finalize();

private:
    std::array<PipelineStateObj*, kBlendNum> texturePso_;
    std::unique_ptr<DxCommand> dxCommand_;

    std::unique_ptr<Matrix4x4> fovMa_;

    struct LoadTask {
        std::string directory;
        std::string fileName;
        Model* model                   = nullptr;
        std::function<void(Model*)> callBack = nullptr;
        void Update();
    };

private:
    std::unique_ptr<TaskThread<LoadTask>> loadThread_;

    std::unordered_map<std::string, std::unique_ptr<ModelMeshData>> modelLibrary_;
    std::unordered_map<std::string, std::vector<Material3D>> defaultMaterials_;

public:
    void pushBackDefaultMaterial(const std::string& key, Material3D material);
};
