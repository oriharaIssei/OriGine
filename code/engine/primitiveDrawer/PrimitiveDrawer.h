#pragma once

#include <System.h>

#include "directX12/shaderManager/ShaderManager.h"

#include <directX12/PipelineStateObj.h>

#include "directX12/buffer/CameraTransform.h"
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/Material.h"
#include "directX12/buffer/Object3dMesh.h"
#include "directX12/buffer/Transform.h"

#include "directX12/dxCommand/DxCommand.h"

#include <array>

#include <memory>
#include <wrl.h>

#include <stdint.h>
#include <Vector3.h>

class SphereObject;
class PrimitiveDrawer{
	friend class SphereObject;
public:
	static void Init();
	static void Finalize();

	static void Line(const Vector3& p0,const Vector3& p1,const IConstantBuffer<Transform>& Transform,const IConstantBuffer<CameraTransform>& viewProj,const IConstantBuffer<Material>* material);
	static void Triangle(const Vector3 &p0,const Vector3 &p1,const Vector3 &p2,const IConstantBuffer<Transform>&Transform,const IConstantBuffer<CameraTransform>&viewProj,const IConstantBuffer<Material>*material);
	static void Quad(const Vector3 &p0,const Vector3 &p1,const Vector3 &p2,const Vector3 &p3,const IConstantBuffer<Transform>&Transform,const IConstantBuffer<CameraTransform>&viewProj,const IConstantBuffer<Material>*material);

	static void ResetInstanceVal(){
		lineInstanceVal_ = 0; triangleInstanceVal_ = 0; quadInstanceVal_ = 0;
	};
private:
	static void CreatePso(System *system = System::getInstance());
private:
	static std::unique_ptr<DxCommand> dxCommand_;

	static std::array<PipelineStateObj *,kBlendNum> trianglePso_;
	static std::array<std::string,kBlendNum> trianglePsoKeys_;

	static std::array<PipelineStateObj *,kBlendNum> linePso_;
	static std::array<std::string,kBlendNum> linePsoKeys_;

	static std::unique_ptr<PrimitiveObject3dMesh> lineMesh_;
	static uint32_t lineInstanceVal_;

	static std::unique_ptr<PrimitiveObject3dMesh> triangleMesh_;
	static uint32_t triangleInstanceVal_;

	static std::unique_ptr<PrimitiveObject3dMesh> quadMesh_;
	static uint32_t quadInstanceVal_;

	static BlendMode currentBlendMode_;
public:
	static void setBlendMode(BlendMode blend){
		currentBlendMode_ = blend;
	}

	static PipelineStateObj *getPrimitivePso(BlendMode blend){ return trianglePso_[static_cast<size_t>(blend)]; }

	static const std::array<std::string,kBlendNum> &getTrianglePsoKeys(){ return trianglePsoKeys_; }
	static const std::array<std::string,kBlendNum> &getLinePsoKeys(){ return linePsoKeys_; }
};