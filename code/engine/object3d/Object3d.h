#pragma once

#include <memory>
#include <vector>

#include <fstream>
#include <sstream>

#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "transform/CameraTransform.h"
#include "directX12/IConstantBuffer.h"
#include "material/Material.h"
#include "transform/Transform.h"

#include "directX12/Object3dMesh.h"

struct Material3D{
	int textureNumber;
	IConstantBuffer<Material>* material;
};

struct Mesh3D{
	std::unique_ptr<IObject3dMesh> meshBuff;
	size_t dataSize  = 0;
	size_t vertSize  = 0;
	size_t indexSize = 0;
};

struct ModelData{
	Mesh3D meshData;
	Material3D materialData;
};

class ModelManager;
class Object3d{
	friend class ModelManager;
public:
	static Object3d* Create(const std::string& directoryPath,const std::string& filename);
	static void PreDraw();

	static void setBlendMode(BlendMode blend){ currentBlend_ = blend; }
private:
	static BlendMode currentBlend_;

public:
	Object3d() = default;
	~Object3d(){ transform_.Finalize(); }

	IConstantBuffer<Transform> transform_;

	void Draw(const IConstantBuffer<CameraTransform>& view);
private:
	void NotDraw([[maybe_unused]] const IConstantBuffer<CameraTransform>& view){}

	void DrawThis(const IConstantBuffer<CameraTransform>& view);
private:
	std::vector<std::unique_ptr<ModelData>> data_;
	enum class LoadState{
		Loading,
		Loaded,
	};
	LoadState currentState_;
	std::array<std::function<void(const IConstantBuffer<CameraTransform>&)>,2> drawFuncTable_ = {
		[this](const IConstantBuffer<CameraTransform>& view){ NotDraw(view); },
		[this](const IConstantBuffer<CameraTransform>& view){ DrawThis(view); }
	};
public:
	const std::vector<std::unique_ptr<ModelData>>& getData()const{ return data_; }
	void setMaterial(IConstantBuffer<Material>* material,uint32_t index = 0){
		data_[index]->materialData.material = material;
	}
};