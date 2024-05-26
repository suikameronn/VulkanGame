#pragma once

#include"GLFW/glfw3.h"

#include"FileManager.h"
#include"VulkanBase.h"
#include"Controller.h"
#include"Scene.h"

extern GLFWwindow* window;

class GameManager
{
private:
	static GameManager* gameManager;

	bool exit = false;
	std::unique_ptr<Scene> scene;

	GameManager() {};

public:
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
		Controller::GetInstance()->FinishController();
		VulkanBase::GetInstance()->FinishVulkanBase();
		FileManager::GetInstance()->FinishFileManger();
		Storage::GetInstance()->FinishStorage();

		delete gameManager;
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