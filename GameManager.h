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

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 720

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
	int exit = GAME_CONTINUE;
	//ステージのインスタンス
	Scene* scene;

	GameManager() {};

	void setLoadUI();

public:

	const int window_width = WINDOW_WIDTH;
	const int window_height = WINDOW_HEIGHT;
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

	//各クラスの終了処理も行う
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
};