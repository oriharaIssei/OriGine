#include "TitleScene.h"

///engine
#include "Engine.h"
//module
#include "SceneManager.h"
#include "input/Input.h"
//component
#include "sprite/Sprite.h"

TitleScene::TitleScene()
    : IScene("TitleScene") {}

TitleScene::~TitleScene() {}

void TitleScene::Init() {
    input_ = Input::getInstance();

    title_ = std::make_unique<Sprite>();
    title_->Init("resource/Texture/Title.png");
    title_->setAnchorPoint(Vector2(0.5f, 0.5f));
    title_->setPosition(Vector2(1280.0f * 0.5f, 720.0f * 0.3f));

    pushA_ = std::make_unique<Sprite>();
    pushA_->Init("resource/Texture/PushA.png");
    pushA_->setAnchorPoint(Vector2(0.5f, 0.5f));
    pushA_->setPosition(Vector2(1280.0f * 0.5f, 720.0f * 0.7f));
}

void TitleScene::Update() {
    time_ = std::fmod(time_, 3.1415f);
    time_ += Engine::getInstance()->getDeltaTime(); // 時間を進める

    pushA_->setColor(Vector4(1.0f, 1.0f, 1.0f, sinf(time_)));

    title_->Update();
    pushA_->Update();

    if (input_->isTriggerButton(XINPUT_GAMEPAD_A)) {
        SceneManager::getInstance()->changeScene("GameScene");
    }
}

void TitleScene::Draw3d() {
}

void TitleScene::DrawLine() {
}

void TitleScene::DrawSprite() {
    title_->Draw();
    pushA_->Draw();
}

void TitleScene::DrawParticle() {
}
