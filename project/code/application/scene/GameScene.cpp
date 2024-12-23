#include "GameScene.h"
#include "TitleScene.h"
#include "GameClearScene.h"

#include "Engine.h"
#include "SceneManager.h"

#include "application/Bullet/Bullet.h"
#include "application/collision/Collision.h"
#include "application/Enemy/Enemy.h"
#include "application/Player/Player.h"

GameScene::GameScene()
    :IScene("GameScene"){}

GameScene::~GameScene(){}

void GameScene::Init(){
#ifdef _DEBUG
#endif // _DEBUG

    input_ = Input::getInstance();

    materialManager_ = Engine::getInstance()->getMaterialManager();

    player_ = std::make_unique<Player>();
    player_->Init();
    for(size_t i = 0; i < 2; i++){
        std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
        enemy->Init();
        enemyList_.push_back(std::move(enemy));
    }
}

void GameScene::Update(){
#ifdef _DEBUG
    materialManager_->DebugUpdate();
#endif // _DEBUG
    if(!player_->getIsAlive()){
        SceneManager::getInstance()->ChangeScene(std::make_unique<TitleScene>());
        return;
    } else if(enemyList_.empty()){
        SceneManager::getInstance()->ChangeScene(std::make_unique<GameClearScene>());
        return;
    }

    ///========================
    /// 各オブジェクトの更新
    ///========================
    {
        player_->Update(this);

        for(auto& bullet : bulletList_){
            bullet->Update();
        }

        for(auto& enemy : enemyList_){
            enemy->Update();
        }
    }

    ///========================
    /// 衝突判定
    ///========================
    {
        collision_->Update(player_.get(),bulletList_,enemyList_);
    }

    ///========================
    /// オブジェクトの削除
    ///========================
    std::erase_if(bulletList_,[](const std::unique_ptr<Bullet>& bullet){ return !bullet->getIsAlive(); });
    std::erase_if(enemyList_,[](const std::unique_ptr<Enemy>& enemy){ return !enemy->getIsAlive(); });
}

void GameScene::Draw3d(){}

void GameScene::DrawLine(){}

void GameScene::DrawSprite(){
    player_->Draw();
    for(auto& enemy : enemyList_){
        enemy->Draw();
    }
    for(auto& bullet : bulletList_){
        bullet->Draw();
    }
}

void GameScene::DrawParticle(){}

void GameScene::addBullet(std::unique_ptr<Bullet> bullet){
    bulletList_.push_back(std::move(bullet));
}
