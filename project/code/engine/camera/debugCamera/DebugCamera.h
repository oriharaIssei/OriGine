#pragma once

#include "input/Input.h"

#include "transform/CameraTransform.h"

#include <memory>

class DebugCamera{
#pragma region State
	class IState{
	public:
		IState(DebugCamera *host) :host_(host){};
		virtual ~IState(){};

		virtual void Update() = 0;
	protected:
		DebugCamera *host_;
	};
	class Neutral :
		public IState{
	public:
		Neutral(DebugCamera *host) :IState(host){};
		void Update()override;
	};
	class RotationState :
		public IState{
	public:
		RotationState(DebugCamera *host) :IState(host){};
		void Update()override;
	};
	class TranslationState :
		public IState{
		enum TranslationType{
			NONE,
			Z_WHEEL,
			XY_MOUSEMOVE,
			XYZ_ALL
		};
	public:
		TranslationState(DebugCamera *host) :IState(host){};
		void Update()override;
	};
#pragma endregion
public:
	void Init();
	void DebugUpdate();
	void Update();
private:
	Input *input_ = nullptr;
	std::unique_ptr<IState> currentState_ = nullptr;
	CameraTransform cameraBuff_;
public:
    void setViewRotate(const Vec3f& rotate) { cameraBuff_.rotate = rotate; }
	void setViewTranslate(const Vec3f &translate){ cameraBuff_.translate = translate; }
	const CameraTransform &getCameraTransform()const{ return cameraBuff_; }
};
