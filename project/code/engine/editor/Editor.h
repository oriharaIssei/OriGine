#pragma once

#include <functional>
#include <string>
#include <vector>

class IComponent;
class IModule;

class WinApp;

class Editor{
public:
	static Editor* getInstance();

	void Finalize();

	void Begin(const std::string& windowName = "oriGine");
	void End();

private:
	Editor() = default;
	~Editor(){};
	Editor(const Editor&) = delete;
	const Editor& operator=(const Editor&) = delete;

	void MenuBarUpdate();
private:
	WinApp* window_ = nullptr;

	std::vector<std::pair<std::string,std::function<void()>>> menuBarFuncs_;
public:
	void setWinApp(WinApp* _winApp);

	bool addMenuBarUpdate(const std::string& label,std::function<void()> func){
		for(auto& [name,func] : menuBarFuncs_){
			if(name == label){
				return false;
			}
		}
		menuBarFuncs_.push_back({label,func});
		return true;
	}
};