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

struct Model;
class Object3d{
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
	Model* data_;

	std::array<std::function<void(const IConstantBuffer<CameraTransform>&)>,2> drawFuncTable_ = {
		[this](const IConstantBuffer<CameraTransform>& view){ NotDraw(view); },
		[this](const IConstantBuffer<CameraTransform>& view){ DrawThis(view); }
	};
public:
	void SetModel(Model* model){ data_ = model; }
	const Model* getData()const;
	void setMaterial(IConstantBuffer<Material>* material,uint32_t index = 0);
};