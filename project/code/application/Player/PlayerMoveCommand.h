#pragma once

#include "IPlayerCommand.h"

/// <summary>
/// Interface for player move commands.
/// </summary>
class IPlayerMoveCommand
    : public IPlayerCommand {
public:
    IPlayerMoveCommand(Player* _player, float _speed);
    ~IPlayerMoveCommand();

    /// <summary>
    /// Executes the command.
    /// </summary>
    /// <param name="player">The player.</param>
    virtual void Execute() = 0;

protected:
    float speed_ = 0.0f;
};

class PlayerMoveLeftCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveLeftCommand(Player* _player, float _speed);
    ~PlayerMoveLeftCommand();

    void Execute() override;
};
class PlayerMoveRightCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveRightCommand(Player* _player, float _speed);
    ~PlayerMoveRightCommand();

    void Execute() override;
};

class PlayerMoveUpCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveUpCommand(Player* _player, float _speed);
    ~PlayerMoveUpCommand();

    void Execute() override;
};
class PlayerMoveDownCommand
    : public IPlayerMoveCommand {
public:
    PlayerMoveDownCommand(Player* _player, float _speed);
    ~PlayerMoveDownCommand();

    void Execute() override;
};
