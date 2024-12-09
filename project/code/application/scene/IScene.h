#pragma once

#include <string>

#include "audio/Audio.h"
#include "directX12/IConstantBuffer.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"

class RenderTexture;
class DxRtvArray;
class DxSrvArray;

class IScene{
public:
	IScene(const std::string& sceneName);
	virtual ~IScene();

	virtual void Init() = 0;
	virtual void Update() = 0;

	virtual void Draw3d()	    = 0;
	virtual void DrawLine()	    = 0;
	virtual void DrawSprite()   = 0;
	virtual void DrawParticle() = 0;
protected:
	std::shared_ptr<DxRtvArray> sceneRtvArray_;
	std::shared_ptr<DxSrvArray> sceneSrvArray_;
private:
	std::string name_;
};
