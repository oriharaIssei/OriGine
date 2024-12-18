#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "directX12/IConstantBuffer.h"
#include "directX12/Object3dMesh.h"
#include "material/Material.h"
#include "transform/Transform.h"

#include "Matrix4x4.h"
#include "Quaternion.h"

struct Material3D{
	uint32_t textureNumber;
	IConstantBuffer<Material>* material;
};

struct ModelNode{
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<ModelNode> children;
};

struct Mesh3D{
	std::unique_ptr<IObject3dMesh> meshBuff;
	size_t dataSize = 0;
	size_t vertSize = 0;
	size_t indexSize = 0;

	IConstantBuffer<Transform> transform_;
	// 対応するノードの名前
	std::string nodeName;
};

struct ModelMeshData{
	std::vector<Mesh3D> mesh_;
	ModelNode rootNode;
};

struct Model{
	enum class LoadState{
		Loading,
		Loaded,
	};
	LoadState currentState_ = LoadState::Loading;

	ModelMeshData* meshData_;

	using ModelMaterialData = std::vector<Material3D>;
	ModelMaterialData materialData_;

	void setMaterialBuff(int32_t part,IConstantBuffer<Material>* buff){
		materialData_[part].material = buff;
	}
	void setTexture(int32_t part,uint32_t textureNumber){
		materialData_[part].textureNumber = textureNumber;
	}
};

template <typename T> struct Keyframe{
	float time; // キーフレームの時刻
	T value;    // キーフレームの 値
};
using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template <typename T> using AnimationCurve = std::vector<Keyframe<T>>;

struct NodeAnimation{
	AnimationCurve<Vector3> scale;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> translate;
};

struct Animation{
	Animation() = default;
	~Animation() = default;

	float duration; // アニメーション 全体の 時間
	// (秒)
	float currentAnimationTime; // アニメーション の 経過時間
	// 各 Node 名で 管理する
	std::map<std::string,NodeAnimation> nodeAnimations;

	void UpdateTime(float deltaTime){
		// 時間更新
		currentAnimationTime += deltaTime;
		// リピート
		currentAnimationTime = std::fmod(currentAnimationTime,duration);
	}
	Matrix4x4 CalculateNodeLocal(const std::string& nodeName) const;
	/// <summary>
	///  指定時間の 値を 計算し 取得
	/// </summary>
	/// <param name="keyframes"></param>
	/// <param name="time"></param>
	/// <returns></returns>
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes,
						   float time) const;
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes,
							  float time) const;
};

void ApplyAnimationToNodes(const ModelNode& node,
						   const Matrix4x4& parentTransform,
						   const Animation& animation,
						   std::map<std::string,Matrix4x4>& outTransforms);