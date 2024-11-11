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

	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber = std::make_unique<Sprite>(SpriteCommon::getInstance());
		aScoreNumber->Init("resource/Texture/numbers.png");
		aScoreNumber->setAnchorPoint({0.5f,0.5f});
		aScoreNumber->setTextureSize(textureTileSize_);
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

	ImGui::Begin("Score");
	ImGui::InputInt("score",&currentScore_);
	ImGui::End();

#endif // _DEBUG

	//int score = currentScore_;
	//std::vector<int32_t> digits;
	//while(score > 0){
	//	int digit = score % 10;
	//	digits.push_back(digit);
	//	score /= 10;
	//}

	//int32_t numberIndex_ = 0;

	//for(auto& aScoreNumber : numberSprites_){
	//	int digit = digits[numberIndex_];
	//	aScoreNumber->setTextureLeftTop(textureTileSize_ * static_cast<float>(digit));

	//	aScoreNumber->Update();
	//	numberIndex_++;
	//}
	//backgroundSprite_->Update();
}

void Score::Draw(){
	backgroundSprite_->Draw();
	for(auto& aScoreNumber : numberSprites_){
		aScoreNumber->Draw();
	}
}