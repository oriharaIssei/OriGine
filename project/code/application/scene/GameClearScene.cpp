#include "GameClearScene.h"

///engine
#include "Engine.h"
//module
#include "SceneManager.h"
#include "input/Input.h"

//component
#include "sprite/Sprite.h"

GameClearScene::GameClearScene()
    : IScene("GameClear") {}

GameClearScene::~GameClearScene() {}

void GameClearScene::Init() {
    input_ = Input::getInstance();

    text_ = std::make_unique<Sprite>();
    text_->Init("resource/Texture/Clear.png");
    text_->setAnchorPoint(Vec2f(0.5f, 0.5f));
    text_->setTranslate(Vec2f(1280.0f * 0.5f, 720.0f * 0.3f));

    pushA_ = std::make_unique<Sprite>();
    pushA_->Init("resource/Texture/PushA.png");
    pushA_->setAnchorPoint(Vec2f(0.5f, 0.5f));
    pushA_->setTranslate(Vec2f(1280.0f * 0.5f, 720.0f * 0.7f));
}

void GameClearScene::Update() {
    time_ = std::fmod(time_, 3.1415f);
    time_ += Engine::getInstance()->getDeltaTime(); // 時間を進める

    pushA_->setColor(Vector4(1.0f, 1.0f, 1.0f, sinf(time_)));

    text_->Update();
    pushA_->Update();

    if (input_->isTriggerButton(XINPUT_GAMEPAD_A)) {
        SceneManager::getInstance()->changeScene("Title");
    }
    text_->Update();
    pushA_->Update();

    if (input_->isTriggerButton(XINPUT_GAMEPAD_A)) {
        SceneManager::getInstance()->changeScene("Title");
    }
}

void GameClearScene::Draw3d() {}

void GameClearScene::DrawLine() {}

void GameClearScene::DrawSprite() {
    text_->Draw();
    pushA_->Draw();
}

void GameClearScene::DrawParticle() {}
