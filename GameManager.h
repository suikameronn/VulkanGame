#pragma once

#include<chrono>
#include <thread>

#include"VulkanBase.h"
#include"Controller.h"
#include"Scene.h"
#include"FileManager.h"

extern GLFWwindow* window;

class GameManager
{
private:

	int frame = 0;

	static GameManager* gameManager;
	
	const int fps = 60;
	float frameDuration;
	std::chrono::system_clock::time_point  start, end;
	std::chrono::system_clock::time_point  start2, end2;
	long long elapsed;

	bool exit = false;
	Scene* scene;

	GameManager() {};

public:

	const uint32_t limitBoneDataSize = 500;

	static GameManager* GetInstance()
	{
		if (!gameManager)
		{
			gameManager = new GameManager();

			glfwSetWindowUserPointer(window, gameManager);
			glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		}

		return gameManager;
	}

	void FinishInstance()
	{
		FileManager::GetInstance()->FinishFileManger();
		VulkanBase::GetInstance()->FinishVulkanBase();
		Storage::GetInstance()->FinishStorage();
		Controller::GetInstance()->FinishController();

		if (gameManager)
		{
			delete gameManager;
		}
		gameManager = nullptr;
	}

	~GameManager()
	{
		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void initGame();
	void createScene();
	void mainGameLoop();
	void exitScene();
	void FinishGame();
};