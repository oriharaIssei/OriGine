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

	void InitOnGameClear();
	void UpdateOnGameClear();
	void ResetStatus(){
		currentScore_ = 0;
	}
private:
	Vector2 tileSize_;
	Vector2 textureTileSize_;
	Vector2 tile2tileSpacing_;
	Vector2 numbersLtPos_;

	Vector2 textPos_;
	Vector2 textSize_;

	Vector2 textPosOnGameClear_;
	Vector2 textSizeOnGameClear_;
	Vector2 numbersLtPosOnGameClear_;
	Vector2 tileSizeOnGameClear_;
	Vector2 tile2tileSpacingOnGameClear_;

	std::unique_ptr<Sprite> scoreTextSprite_;
	std::array<std::unique_ptr<Sprite>,5> numberSprites_;
	std::array<int32_t,5> digits_;

	int32_t currentScore_;
public:
	void AddScore(int32_t add){ currentScore_ += add; }
};