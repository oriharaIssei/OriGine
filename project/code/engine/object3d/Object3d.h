#pragma once

#include <memory>
#include <vector>

#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "material/Material.h"
#include "model/Model.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

class Object3d{
public:
	static void PreDraw();

    void Init(const std::string& directoryPath, const std::string& filename);
    void Update();
private:
	static BlendMode currentBlend_;

public:
	Object3d() = default;
	~Object3d(){}

	Transform transform_;

	void Draw();
private:
	void NotDraw(){}

	void DrawThis();
private:
	std::unique_ptr<Model> data_;

	std::array <std::function<void()>,2> drawFuncTable_ = {
		[this](){ NotDraw(); },
		[this](){ DrawThis(); }
	};
public:
	const Model* getModel()const{ return data_.get(); }
	void setMaterial(IConstantBuffer<Material>* material,uint32_t index = 0);
};
