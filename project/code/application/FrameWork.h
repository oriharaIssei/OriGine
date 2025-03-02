#pragma once

class Engine;
class GlobalVariables;

class FrameWork{
public:
	FrameWork();
	virtual ~FrameWork();

	virtual void Init();
	virtual void Finalize();

	virtual void Run() = 0;
protected:
	bool isEndRequest_ = false;
	Engine* engine_ = nullptr;
	GlobalVariables* variables_ = nullptr;
};