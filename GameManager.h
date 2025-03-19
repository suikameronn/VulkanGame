#pragma once

#include<chrono>
#include <thread>

#include"VulkanBase.h"
#include"Controller.h"
#include"Scene.h"
#include"FileManager.h"

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
	bool exit = false;
	//�X�e�[�W�̃C���X�^���X
	Scene* scene;

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

	//�e�N���X�̏I���������s��
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

	//�Q�[���̃t���[�����[�g�̐ݒ��X�e�[�W�̓ǂݍ��݂��s��
	void initGame();
	//lua����X�e�[�W�̗l�q��ǂݍ���
	void createScene();
	//���C�����[�v
	void mainGameLoop();
	//�X�e�[�W����o��
	void exitScene();
	//�Q�[���S�̂̏I������
	void FinishGame();
};