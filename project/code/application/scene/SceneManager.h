#pragma once

#include <memory>
#include <unordered_map>

#include <string>

class IScene;

class RenderTexture;
class DxRtvArray;
class DxSrvArray;

class SceneManager{
public:
	static SceneManager* getInstance();

	void Init();
	void Finalize();

	void Update();
	void Draw();
private:
	SceneManager();
	~SceneManager();
	SceneManager(const SceneManager&) = delete;
	SceneManager* operator=(const SceneManager&) = delete;
private:
	std::unique_ptr<IScene> currentScene_;

	std::unique_ptr<RenderTexture> sceneView_;
	std::shared_ptr<DxRtvArray> sceneViewRtvArray_;
	std::shared_ptr<DxSrvArray> sceneViewSrvArray_;
public:
	void ChangeScene(std::unique_ptr<IScene> next);
};