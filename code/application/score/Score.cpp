#include "Score.h"

#include "globalVariables/GlobalVariables.h"

Score* Score::getInstance(){
	static Score instance{};
	return &instance;
}

void Score::Init(){
	GlobalVariables* variables = GlobalVariables::getInstance();
	variables->addValue("Game","Score","tileSize_",tileSize_);
	variables->addValue("Game","Score","tile2tileSpacing_",tile2tileSpacing_);
	variables->addValue("Game","Score","numbersLtPos_",numbersLtPos_);

	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber = std::make_unique<Sprite>(SpriteCommon::getInstance());
		aScoreNumber->Init("resource/Texture/numbers.png");
		aScoreNumber->setAnchorPoint({0.5f,0.5f});
		aScoreNumber->setTextureLeftTop(tileSize_);
		aScoreNumber->setTextureSize({32.0f,32.0f});
	}
	for(size_t i = 0; i < numberSprites_.size(); i++){
		numberSprites_[i]->setPosition({
			numbersLtPos_.x + (tileSize_.x + tile2tileSpacing_.x) * i,
			numbersLtPos_.y + (tile2tileSpacing_.y * 0.5f)
									   });
	}

	backgroundSprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
	backgroundSprite_->Init("resource/white1x1.png");
}

void Score::Finalize(){
	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber.reset();
	}
	backgroundSprite_.reset();
}

void Score::Update(){
#ifdef _DEBUG
	for(size_t i = 0; i < numberSprites_.size(); i++){
		numberSprites_[i]->setPosition({
			numbersLtPos_.x + (tileSize_.x + tile2tileSpacing_.x) * i,
			numbersLtPos_.y + (tile2tileSpacing_.y * 0.5f)
									   });
		numberSprites_[i]->setSize(tileSize_);
	}
#endif // _DEBUG

	int32_t numberIndex_ = 0;
	for(auto& aScoreNumber : numberSprites_){
		int bit = (currentScore_ >> ((numberSprites_.size() - 1) - numberIndex_)) & 1;
		aScoreNumber->setTextureLeftTop(tileSize_ * static_cast<float>(bit));
		aScoreNumber->Update();
		numberIndex_++;
	}
	backgroundSprite_->Update();
}

void Score::Draw(){
	backgroundSprite_->Draw();
	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber->Draw();
	}
}
