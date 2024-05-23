#pragma once

#include"GLFW/glfw3.h"

#include"FileManager.h"
#include"VulkanBase.h"

extern GLFWwindow* window;

class GameManager
{
private:
	static GameManager* gameManager;
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

	void FinishApp()
	{
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


	void GameLoop();
};