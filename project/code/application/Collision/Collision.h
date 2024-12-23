#pragma once

#include <memory>
#include <vector>

class Player;
class Bullet;
class Enemy;

class Collision {
public:
    Collision();
    ~Collision();

    void Update(Player* _player, std::vector<std::unique_ptr<Bullet>>& _bulletList, std::vector<std::unique_ptr<Enemy>>& _enemyList);

private:
    void CheckCollisionBulletAndEnemy(std::vector<std::unique_ptr<Bullet>>& _bulletList, std::vector<std::unique_ptr<Enemy>>& _enemyList);
    void CheckCollisionPlayerAndEnemy(Player* _player, std::vector<std::unique_ptr<Enemy>>& _enemyList);
};
