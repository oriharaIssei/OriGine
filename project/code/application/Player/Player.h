#pragma once

#include <list>
#include <memory>

struct Vector2;
class Sprite;

class IPlayerCommand;

class Player {
public:
    Player();
    ~Player();

    void Init();
    void Update();
    void Draw();

private:
    /// <summary>
    /// 入力による command の生成
    /// </summary>
    void InputHandle();

    bool UndoCommand();
    bool RedoCommand();

private:
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<IPlayerCommand> currentCommand_;
    std::list<std::unique_ptr<IPlayerCommand>>::iterator currentCommandItr_;
    std::list<std::unique_ptr<IPlayerCommand>> commandHistory_;
    float speed_ = 12.0f;

public:
    const Vector2& getPos() const;
    const void setPos(const Vector2& pos);
};
