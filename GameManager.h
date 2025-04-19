#pragma once

#include<chrono>
#include <thread>

#include "lua/include/lua.hpp"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"

#include"VulkanBase.h"
#include"Controller.h"
#include"Scene.h"
#include"FileManager.h"
#include"UI.h"

extern GLFWwindow* window;

//fps����Ȃǂ̃Q�[���S�̂̐����S������
class GameManager
{
private:

	static GameManager* gameManager;

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

	void setLoadUI();

public:

	const int window_width = 800;
	const int window_height = 600;
	const glm::mat4 uiProjection = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f, -1.0f, 1.0f);

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
		FileManager::FinishFileManger();
		VulkanBase::FinishVulkanBase();
		Storage::FinishStorage();
		Controller::FinishController();

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
	//�ǂݎ�����f�[�^�͂��̂܂܂ɃQ�[�����X�^�[�g������
	void RestartGame();

	//���[�hUI��\��
	void drawLoading(bool& loadFinish);
};