#include "Collision.h"

#include "Vector2.h"
#include "application/Bullet/Bullet.h"
#include "application/Enemy/Enemy.h"
#include "application/Player/Player.h"

Collision::Collision() {
}

Collision::~Collision() {
}

void Collision::Update(Player* _player, std::vector<std::unique_ptr<Bullet>>& _bulletList, std::vector<std::unique_ptr<Enemy>>& _enemyList) {
    CheckCollisionPlayerAndEnemy(_player, _enemyList);
    CheckCollisionBulletAndEnemy(_bulletList, _enemyList);
}

void Collision::CheckCollisionBulletAndEnemy(std::vector<std::unique_ptr<Bullet>>& _bulletList, std::vector<std::unique_ptr<Enemy>>& _enemyList) {
    if (_bulletList.empty() || _enemyList.empty()) {
        return;
    }
    for (auto& bullet : _bulletList) {
        if (bullet->getIsAlive() == false) {
            continue;
        }
        for (auto& enemy : _enemyList) {
            if (enemy->getIsAlive() == false) {
                continue;
            }
            if ((enemy->getPosition() - bullet->getPosition()).lengthSq() < (bullet->getRadius() + enemy->getRadius()) * (bullet->getRadius() + enemy->getRadius())) {
                bullet->OnCollision();
                enemy->OnCollision();
            }
        }
    }
}

void Collision::CheckCollisionPlayerAndEnemy(Player* _player, std::vector<std::unique_ptr<Enemy>>& _enemyList) {
    if (_player->getIsAlive() == false || _enemyList.empty()) {
        return;
    }
    for (auto& enemy : _enemyList) {
        if (enemy->getIsAlive() == false) {
            continue;
        }
        if ((enemy->getPosition() - _player->getPosition()).lengthSq() < (_player->getRadius() + enemy->getRadius()) * (_player->getRadius() + enemy->getRadius())) {
            _player->OnCollision();
            enemy->OnCollision();
        }
    }
}
