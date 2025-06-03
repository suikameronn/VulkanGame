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

extern GLFWwindow* window;

//fps制御などのゲーム全体の制御を担当する
class GameManager
{
private:

	static GameManager* gameManager;

	int window_width, window_height;
	glm::mat4 uiProjection;

	//最大フレームレート
	const int fps = 60;
	//最大フレームレート時のフレームの経過時間
	float frameDuration;
	//時間の計測用変数
	std::chrono::system_clock::time_point  start, end;
	//計測したフレーム時間
	long long elapsed;

	//ゲームの終了フラッグ
	int exit = GAME_CONTINUE;
	//ステージのインスタンス
	Scene* scene;

	GameManager() {};

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

	//各クラスの終了処理も行う
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

	//ゲームのフレームレートの設定やステージの読み込みを行う
	void initGame();
	//luaからステージの様子を読み込む
	bool createScene();
	//メインループ
	void mainGameLoop();
	//ステージから出る
	void exitScene();
	//ゲーム全体の終了処理
	void FinishGame();
	//読み取ったデータはそのままにゲームリスタートさせる
	void RestartGame();

	//ロードUIを表示
	void drawLoading(bool& loadFinish);

	int getWindowWidth() { return window_width; }
	int getWindowHeight() { return window_height; }
	glm::mat4 getUIProjection() { return uiProjection; }
};