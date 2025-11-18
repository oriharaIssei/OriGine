#pragma once

/// stl
// memory
#include <functional>
#include <memory>
// contiainer
#include <array>
#include <unordered_map>
// basic class
#include <string>

/// engine
// assetes

struct Model;
struct ModelNode;
struct ModelMeshData;
struct TexturedMaterial;

// dx12Object
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"


// math
#include "Matrix4x4.h"

class ModelManager {

public:
    static ModelManager* GetInstance();

    std::shared_ptr<Model> Create(
        const std::string& directoryPath,
        const std::string& filename,
        std::function<void(Model*)> callBack = nullptr);

    void Initialize();
    void Finalize();

private:
    std::array<PipelineStateObj*, kBlendNum> texturePso_;
    std::unique_ptr<DxCommand> dxCommand_;

    std::unique_ptr<Matrix4x4> fovMa_;

    struct LoadTask {
        std::string directory;
        std::string fileName;
        std::shared_ptr<Model> model = nullptr;

        std::function<void(Model*)> callBack = nullptr;
        void Update();
    };

private:
    // std::unique_ptr<TaskThread<LoadTask>> loadThread_;

    std::unordered_map<std::string, std::unique_ptr<ModelMeshData>> modelLibrary_;
    std::unordered_map<ModelMeshData*, std::vector<TexturedMaterial>> defaultMaterials_;

public:
    void pushBackDefaultMaterial(ModelMeshData* key, TexturedMaterial material);
    ModelMeshData* GetModelMeshData(const std::string& directoryPath, const std::string& filename);
    const std::vector<TexturedMaterial>& GetDefaultMaterials(ModelMeshData* key) const;
    const std::vector<TexturedMaterial>& GetDefaultMaterials(const std::string& directoryPath, const std::string& filename) const;
};
