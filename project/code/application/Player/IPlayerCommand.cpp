#include "IPlayerCommand.h"

#include "Player.h"

IPlayerCommand::IPlayerCommand(Player* _player)
    : player_(_player) {}

IPlayerCommand::~IPlayerCommand() {}
