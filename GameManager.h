#pragma once

#include"FileManager.h"
#include"Controller.h"
#include"VulkanBase.h"
#include"Scene.h"

class GameManager
{
private:
	std::shared_ptr<FileManager> fileManager;//ファイルを読み取る
	std::shared_ptr<VulkanBase> vulkanBase;//vulkanの機能を使う
	std::shared_ptr<Controller> controller;//コントローラ

	GLFWwindow* window;

public:
	GameManager() {};
	GameManager(std::shared_ptr<FileManager> f, std::shared_ptr<VulkanBase> v
		,std::shared_ptr<Controller> c,GLFWwindow* w)
	{
		window = w;
		fileManager = f;
		vulkanBase = v;
		controller = c;

		glfwSetWindowUserPointer(window, controller.get());
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void GameLoop();
};