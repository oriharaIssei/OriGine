#pragma once

///stl
//memory
#include <memory>
//container
#include <list>
//string
#include <string>

///local
//parent
#include "IScene.h"
//camera
#include "camera/debugCamera/DebugCamera.h"

class ModelViewScene
    : public IScene {
public:
    ModelViewScene();
    ~ModelViewScene();

    void Init();
    void Update();

    void Draw3d();
    void DrawLine();
    void DrawSprite();
    void DrawParticle();

private:
    static std::list<std::pair<std::string, std::string>> modelFileList_;
    static std::list<std::pair<std::string, std::string>> textureFileList_;
    //DebugObjects
    std::unique_ptr<DebugCamera> debugCamera_;
    // editObject
    std::unique_ptr<Object3d> editObject_ = nullptr;

    //ground
    std::unique_ptr<Object3d> ground_ = nullptr;
};
