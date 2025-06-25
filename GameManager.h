#pragma once

#include<chrono>
#include <thread>
#include <any>

#include "lua/include/lua.hpp"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"

#include"VulkanBase.h"
#include"Controller.h"
#include"Scene.h"
#include"FileManager.h"
#include"FontManager.h"
#include"UI.h"

#include"ThreadPool.h"

#include"renderdoc_app.h"

extern GLFWwindow* window;

//fps����Ȃǂ̃Q�[���S�̂̐����S������
class GameManager
{
private:

	static GameManager* gameManager;

	int window_width, window_height;
	glm::mat4 uiProjection;

	//�ő�t���[�����[�g
	const int fps = 60;
	//�ő�t���[�����[�g���̃t���[���̌o�ߎ���
	float frameDuration;
	//���Ԃ̌v���p�ϐ�
	std::chrono::system_clock::time_point  start, end;
	//�v�������t���[������
	long long elapsed;

	//�Q�[���̏I���t���b�O
	int exit = GAME_CONTINUE;
	//�X�e�[�W�̃C���X�^���X
	Scene* scene;

	GameManager() {};

	pRENDERDOC_GetAPI getApi;
	RENDERDOC_API_1_0_0* rdoc_api;

	void setLoadUI();

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

	//�e�N���X�̏I���������s��
	void FinishInstance()
	{
		FontManager::FinishInstance();
		FileManager::FinishFileManger();
		VulkanBase::FinishVulkanBase();
		Storage::FinishStorage();
		Controller::FinishController();
		ThreadPool::FinishThreadPool();

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

	//�Q�[���̃t���[�����[�g�̐ݒ��X�e�[�W�̓ǂݍ��݂��s��
	void initGame();
	//lua����X�e�[�W�̗l�q��ǂݍ���
	bool createScene();
	//���C�����[�v
	void mainGameLoop();
	//�X�e�[�W����o��
	void exitScene();
	//�Q�[���S�̂̏I������
	void FinishGame();

	//���[�hUI��\��
	void drawLoading(bool& loadFinish);

	int getWindowWidth() { return window_width; }
	int getWindowHeight() { return window_height; }
	glm::mat4 getUIProjection() { return uiProjection; }

	void renderDoc()
	{
		rdoc_api = nullptr;
		HMODULE module = GetModuleHandleA("renderdoc.dll");
		if (module)
		{
			getApi = (pRENDERDOC_GetAPI)GetProcAddress(module, "RENDERDOC_GetAPI");
			if (getApi)
			{
				getApi(eRENDERDOC_API_Version_1_6_0, (void**)&rdoc_api);
			}
		}
	}

	void startCapture()
	{
		if (rdoc_api)
		{
			rdoc_api->StartFrameCapture(nullptr, nullptr);
		}
	}

	void endCapture()
	{
		if (rdoc_api)
		{
			rdoc_api->EndFrameCapture(nullptr, nullptr);
		}
	}
};