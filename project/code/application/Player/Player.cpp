#include "Player.h"
// commands
#include "IPlayerCommand.h"
#include "PlayerMoveCommand.h"

// input
#include "input/Input.h"

// sprite
#include "sprite/Sprite.h"
#include "sprite/SpriteCommon.h"

Player::Player() {}

Player::~Player() {}

void Player::Init() {
    sprite_.reset(SpriteCommon::getInstance()->Create("resource/Texture/white1x1.png"));
    // TODO マルチスレッドでの textureSize 同期
    sprite_->setTextureSize({1.0f, 1.0f});
    // 移動分と サイズを 同じにする
    sprite_->setSize({speed_, speed_});
    // セットした内容を 反映
    sprite_->Update();
}

void Player::Update() {
    // command 初期化
    currentCommand_ = nullptr;

    // inputによるcommandの生成
    InputHandle();

    // コマンドの適応
    if (currentCommand_) {
        if(!commandHistory_.empty()){
             auto backPreItr = std::prev(commandHistory_.end());
            if (currentCommandItr_ != backPreItr) {
                // undo,redo の後に command を追加した場合、それ以降の command を削除する
                commandHistory_.erase(std::next(currentCommandItr_), backPreItr);
             }
        }

        commandHistory_.push_back(std::move(currentCommand_));
        currentCommandItr_ = std::prev(commandHistory_.end());
        currentCommandItr_->get()->Execute();
    }
}

void Player::Draw() {
    sprite_->Draw();
}

void Player::InputHandle() {
    // 入力取得
    Input* input = Input::getInstance();

    // UnDo Redo チェック

    if (input->isPressKey(DIK_LCONTROL)) {       // Ctrl を押しながら
        if (input->isTriggerKey(DIK_Z)) {        // Z を押したら
            if (input->isPressKey(DIK_LSHIFT)) { // Shift + Z を押したら
                if (RedoCommand()) {             // ReDo
                    // 他のコマンドを実行しないように return する
                    return;
                }
            } else {                 // Shift を押していなければ
                if (UndoCommand()) { // Undo
                    // 他のコマンドを実行しないように return する
                    return;
                }
            }
        } else if (input->isTriggerKey(DIK_Y)) { // Y を押したら
            if (RedoCommand()) {                 // ReDo
                // 他のコマンドを実行しないように return する
                return;
            }
        }
    }

    if (input->isTriggerKey(DIK_W)) {
        currentCommand_ = std::make_unique<PlayerMoveUpCommand>(this, speed_);
    } else if (input->isTriggerKey(DIK_S)) {
        currentCommand_ = std::make_unique<PlayerMoveDownCommand>(this, speed_);
    } else if (input->isTriggerKey(DIK_A)) {
        currentCommand_ = std::make_unique<PlayerMoveLeftCommand>(this, speed_);
    } else if (input->isTriggerKey(DIK_D)) {
        currentCommand_ = std::make_unique<PlayerMoveRightCommand>(this, speed_);
    }
}

bool Player::UndoCommand() {
    if (commandHistory_.empty()) {
        return false;
    }
    if (currentCommandItr_ == commandHistory_.begin()) {
        return false;
    }
    currentCommandItr_ = --currentCommandItr_;
    currentCommandItr_->get()->Execute();
    return true;
}

bool Player::RedoCommand() {
    if (commandHistory_.empty()) {
        return false;
    }
    if (currentCommandItr_ == --commandHistory_.end()) {
        return false;
    }
    currentCommandItr_ = ++currentCommandItr_;
    currentCommandItr_->get()->Execute();
    return true;
}

const Vector2& Player::getPos() const {
    return sprite_->getPosition();
}

const void Player::setPos(const Vector2& pos) {
    sprite_->setPosition(pos);
}
