#pragma once
#include "IGameObject.h"

#include "sprite/Sprite.h"

#include "Vector2.h"

class SpriteObject :
	public IGameObject{
public:
	~SpriteObject()override{};
	void Init([[maybe_unused]] const std::string &directryPath,const std::string &objectName)override;
	void Update()override;
	void Draw(const ViewProjection &viewProj)override;
private:
	std::unique_ptr<Sprite> sprite_;

	Vector2 pos_,size_;
	Vector4 color_;
};