
#pragma once

#include "Matrix4x4.h"
#include "Thread/Thread.h"
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

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

    std::unique_ptr<Model> Create(const std::string& directoryPath, const std::string& filename);

    void Init();
    void Finalize();

private:
    std::array<PipelineStateObj*, kBlendNum> texturePso_;
    std::unique_ptr<DxCommand> dxCommand_;

    std::unique_ptr<Matrix4x4> fovMa_;

    struct LoadTask {
        std::string directory;
        std::string fileName;
        Model* model = nullptr;
        void Update();
    };

private:
    std::unique_ptr<TaskThread<LoadTask>> loadThread_;

    std::unordered_map<std::string, std::unique_ptr<ModelMeshData>> modelLibrary_;
    std::unordered_map<ModelMeshData*, std::vector<Material3D>> defaultMaterials_;

public:
    void pushBackDefaultMaterial(ModelMeshData* key, Material3D material);
};
