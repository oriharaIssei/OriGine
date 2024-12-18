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

class AnimationObject3d{
public:
	static std::unique_ptr<AnimationObject3d> Create(const std::string& _modelDirectoryPath,
													 const std::string& _modelFilename);

	static std::unique_ptr<AnimationObject3d> Create(const std::string& _modelDirectoryPath,
													 const std::string& _modelFilename,
													 const std::string& _animationDirectoryPath ,
													 const std::string& _animationFilename );
	// まだ Object3d と 同じで良い
	//	static void PreDraw();

	static void setBlendMode(BlendMode blend){ currentBlend_ = blend; }
private:
	static BlendMode currentBlend_;

public:
	AnimationObject3d() = default;
	~AnimationObject3d(){}

	Transform transform_;

	void Update(float deltaTime);
	void Draw();
private:
	void NotDraw(){}

	void DrawThis();
private:
	std::array <std::function<void()>,2> drawFuncTable_ = {
		[this](){ NotDraw(); },
		[this](){ DrawThis(); }
	};

	std::unique_ptr<Model> data_;
	std::unique_ptr<Animation> animation_;
public:
	const Model* getModel()const{ return data_.get(); }
	void setMaterial(IConstantBuffer<Material>* material,uint32_t index = 0);
};