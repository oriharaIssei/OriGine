#include "PlayerMoveCommand.h"

#include "Player.h"
#include "Vector2.h"

///================================================
// IPlayerMoveCommand
///================================================
IPlayerMoveCommand::IPlayerMoveCommand(Player* _player, float _speed)
    : IPlayerCommand(_player), speed_(_speed) {}

IPlayerMoveCommand::~IPlayerMoveCommand() {}
///================================================

///================================================
// PlayerMoveLeftCommand
///================================================
PlayerMoveLeftCommand::PlayerMoveLeftCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player, _speed) {}

PlayerMoveLeftCommand::~PlayerMoveLeftCommand() {}

void PlayerMoveLeftCommand::Execute() {
    Vector2 pos = player_->getPos();
    pos.x -= speed_;
    player_->setPos(pos);
}
///================================================

///================================================
// PlayerMoveRightCommand
///================================================
PlayerMoveRightCommand::PlayerMoveRightCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player, _speed) {}

PlayerMoveRightCommand::~PlayerMoveRightCommand() {}

void PlayerMoveRightCommand::Execute() {
    Vector2 pos = player_->getPos();
    pos.x += speed_;
    player_->setPos(pos);
}
///================================================

///================================================
// PlayerMoveUpCommand
///================================================
PlayerMoveUpCommand::PlayerMoveUpCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player, _speed) {}

PlayerMoveUpCommand::~PlayerMoveUpCommand() {}

void PlayerMoveUpCommand::Execute() {
    Vector2 pos = player_->getPos();
    pos.y -= speed_;
    player_->setPos(pos);
}
///================================================

///================================================
// PlayerMoveDownCommand
///================================================
PlayerMoveDownCommand::PlayerMoveDownCommand(Player* _player, float _speed)
    : IPlayerMoveCommand(_player, _speed) {}

PlayerMoveDownCommand::~PlayerMoveDownCommand() {}

void PlayerMoveDownCommand::Execute() {
    Vector2 pos = player_->getPos();
    pos.y += speed_;
    player_->setPos(pos);
}
///================================================
