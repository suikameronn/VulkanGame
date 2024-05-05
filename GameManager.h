#pragma once

#include"FileManager.h"
#include"Controller.h"
#include"VulkanBase.h"
#include"Scene.h"

class GameManager
{
private:
	std::shared_ptr<FileManager> fileManager;//�t�@�C����ǂݎ��
	std::shared_ptr<VulkanBase> vulkanBase;//vulkan�̋@�\���g��
	std::shared_ptr<Controller> controller;//�R���g���[��

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