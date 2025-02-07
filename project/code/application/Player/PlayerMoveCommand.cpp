#include "PlayerMoveCommand.h"

#include "Player.h"
#include "Vector2.h"

///================================================
// IPlayerMoveCommand
///================================================
IPlayerMoveCommand::IPlayerMoveCommand(Player* _player)
    : IPlayerCommand(_player) {}

IPlayerMoveCommand::~IPlayerMoveCommand() {}

void IPlayerMoveCommand::Execute() {
    player_->setPos(pos_);
}
///================================================

///================================================
// PlayerMoveLeftCommand
///================================================
PlayerMoveLeftCommand::PlayerMoveLeftCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player) {
    pos_ = player_->getPos();
    pos_.x -= _speed;
}

PlayerMoveLeftCommand::~PlayerMoveLeftCommand() {}
///================================================

///================================================
// PlayerMoveRightCommand
///================================================
PlayerMoveRightCommand::PlayerMoveRightCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player) {
    pos_ = player_->getPos();
    pos_.x += _speed;
}

PlayerMoveRightCommand::~PlayerMoveRightCommand() {}

///================================================

///================================================
// PlayerMoveUpCommand
///================================================
PlayerMoveUpCommand::PlayerMoveUpCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player) {
    pos_ = player_->getPos();
    pos_.y -= _speed;
}

PlayerMoveUpCommand::~PlayerMoveUpCommand() {}

///================================================

///================================================
// PlayerMoveDownCommand
///================================================
PlayerMoveDownCommand::PlayerMoveDownCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player) {
    pos_ = player_->getPos();
    pos_.y += _speed;
}

PlayerMoveDownCommand::~PlayerMoveDownCommand() {}

///================================================
