#pragma once
class Player;

class IPlayerCommand {
public:
    IPlayerCommand(Player* _player);
    ~IPlayerCommand();

    virtual void Execute() = 0;

protected:
    Player* player_;
};
