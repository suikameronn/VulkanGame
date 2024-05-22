#pragma once

#include"GLFW/glfw3.h"

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

	

	~GameManager()
	{
		delete gameManager;
		gameManager = nullptr;
	}


	void GameLoop();
};