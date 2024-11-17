#include "Score.h"

#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

Score* Score::getInstance(){
	static Score instance{};
	return &instance;
}

void Score::Init(){
	GlobalVariables* variables = GlobalVariables::getInstance();
	variables->addValue("Game","Score","tileSize_",tileSize_);
	variables->addValue("Game","Score","textureTileSize_",textureTileSize_);
	variables->addValue("Game","Score","tile2tileSpacing_",tile2tileSpacing_);
	variables->addValue("Game","Score","numbersLtPos_",numbersLtPos_);
	variables->addValue("Game","Score","scoreTextSize_",textSize_);
	variables->addValue("Game","Score","scoreTextPos_",textPos_);

	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber = std::make_unique<Sprite>(SpriteCommon::getInstance());
		aScoreNumber->Init("resource/Texture/numbers.png");
		aScoreNumber->setAnchorPoint({0.5f,0.5f});
		aScoreNumber->setTextureSize(textureTileSize_);
		aScoreNumber->setSize(tileSize_);
	}
	for(size_t i = 0; i < numberSprites_.size(); i++){
		numberSprites_[i]->setPosition({
			numbersLtPos_.x + (tileSize_.x + tile2tileSpacing_.x) * i,
			numbersLtPos_.y + (tile2tileSpacing_.y * 0.5f)
									   });
	}

	scoreTextSprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
	scoreTextSprite_->Init("resource/Texture/scoreText.png");
	scoreTextSprite_->setTextureSize({100.0f,36.0f});
	scoreTextSprite_->setAnchorPoint({0.5f,0.5f});
	scoreTextSprite_->setSize(textSize_);
	scoreTextSprite_->setPosition(textPos_);
	scoreTextSprite_->Update();
}

void Score::Finalize(){
	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber.reset();
	}
	scoreTextSprite_.reset();
}

void Score::Update(){
	int score = currentScore_;

	for(int32_t i = 0; i < 5; i++){
		int digit = score % 10;
		digits_[i] = digit;
		score /= 10;
	}
	std::reverse(digits_.begin(),digits_.end());

	int32_t numberIndex_ = 0;

	for(auto& aScoreNumber : numberSprites_){
		int digit = digits_[numberIndex_];
		aScoreNumber->setTextureLeftTop(textureTileSize_ * static_cast<float>(digit));

		aScoreNumber->Update();
		numberIndex_++;
	}

	scoreTextSprite_->setPosition(textPos_);
	scoreTextSprite_->setSize(textSize_);
	scoreTextSprite_->Update();
}

void Score::Draw(){
	scoreTextSprite_->Draw();
	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber->Draw();
	}
}