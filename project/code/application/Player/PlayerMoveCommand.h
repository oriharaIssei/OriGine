#pragma once

#include "IPlayerCommand.h"

#include "Vector2.h"

/// <summary>
/// Interface for player move commands.
/// </summary>
class IPlayerMoveCommand
    : public IPlayerCommand {
public:
    IPlayerMoveCommand(Player* _player);
    ~IPlayerMoveCommand();

    /// <summary>
    /// Executes the command.
    /// </summary>
    void Execute() override;

protected:
    Vector2 pos_;
};

class PlayerMoveLeftCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveLeftCommand(Player* _player, float _speed);
    ~PlayerMoveLeftCommand();

};
class PlayerMoveRightCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveRightCommand(Player* _player, float _speed);
    ~PlayerMoveRightCommand();

};

class PlayerMoveUpCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveUpCommand(Player* _player, float _speed);
    ~PlayerMoveUpCommand();

};
class PlayerMoveDownCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveDownCommand(Player* _player, float _speed);
    ~PlayerMoveDownCommand();

};
