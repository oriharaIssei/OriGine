#include "TitleScene.h"

//module
#include "input/Input.h"
#include "SceneManager.h"
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
}

void TitleScene::Update() {
    title_->Update();

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
}

void TitleScene::DrawParticle() {
}
