#pragma once

#include<chrono>
#include <thread>

#include"VulkanBase.h"
#include"Controller.h"
#include"Scene.h"
#include"FileManager.h"

extern GLFWwindow* window;

//fps制御などのゲーム全体の制御を担当する
class GameManager
{
private:

	static GameManager* gameManager;

	//最大フレームレート
	const int fps = 60;
	//最大フレームレート時のフレームの経過時間
	float frameDuration;
	//時間の計測用変数
	std::chrono::system_clock::time_point  start, end;
	//計測したフレーム時間
	long long elapsed;

	//ゲームの終了フラッグ
	bool exit = false;
	//ステージのインスタンス
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

	//各クラスの終了処理も行う
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

	//ゲームのフレームレートの設定やステージの読み込みを行う
	void initGame();
	//luaからステージの様子を読み込む
	void createScene();
	//メインループ
	void mainGameLoop();
	//ステージから出る
	void exitScene();
	//ゲーム全体の終了処理
	void FinishGame();
};