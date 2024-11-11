#pragma once

#include <array>
#include <memory>

#include "sprite/Sprite.h"

class Score{
public:
	static Score* getInstance();

	Score() = default;
	~Score(){};

	void Init();
	void Finalize();
	void Update();
	void Draw();
private:
	Vector2 tileSize_;
	Vector2 tile2tileSpacing_;
	Vector2 numbersLtPos_;

	std::array<std::unique_ptr<Sprite>,5> numberSprites_;
	std::unique_ptr<Sprite> backgroundSprite_;

	int32_t currentScore_;
public:
	void AddScore(int32_t add){ currentScore_ += add; }
};

