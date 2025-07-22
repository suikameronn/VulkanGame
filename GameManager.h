#pragma once

#include<chrono>
#include <thread>
#include <any>

#include "lua/include/lua.hpp"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"

#include"DescriptorSetFactory.h"
#include"FrameBufferFactory.h"
#include"GpuBufferFactory.h"
#include"DescriptorSetLayoutFactory.h"
#include"PipelineLayoutFactory.h"
#include"PipelineFactory.h"
#include"RenderPassFactory.h"
#include"ShaderFactory.h"

#include"ECSManager.h"

//fps制御などのゲーム全体の制御を担当する
class GameManager
{
private:

	static GameManager* gameManager;

	std::shared_ptr<VulkanCore> vulkanCore;

	//ECSマネージャー
	std::shared_ptr<ECSManager> ecsManager;

	//ビルダー

	std::shared_ptr<DescriptorSetBuilder> descriptorSetBuilder;
	std::shared_ptr<FrameBufferBuilder> frameBufferBuilder;
	std::shared_ptr<GpuBufferBuilder> bufferBuilder;
	std::shared_ptr<DescriptorSetLayoutBuilder> descriptorSetLayoutBuilder;
	std::shared_ptr<PipelineLayoutBuilder> pipelineLayoutBuilder;
	std::shared_ptr<PipelineBuilder> pipelineBuilder;
	std::shared_ptr<RenderPassBuilder> renderPassBuilder;
	
	//ファクトリー

	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;
	std::shared_ptr<FrameBufferFactory> frameBufferFactory;
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> descriptorSetLayoutFactory;
	std::shared_ptr<PipelineLayoutFactory> pipelineLayoutFactory;
	std::shared_ptr<PipelineFactory> pipelineFactory;
	std::shared_ptr<RenderPassFactory> renderPassFactory;

	//最大フレームレート
	const int fps = 60;
	//最大フレームレート時のフレームの経過時間
	float frameDuration;
	//時間の計測用変数
	std::chrono::system_clock::time_point  start, end;
	//計測したフレーム時間
	long long elapsed;

	GameManager() {};

	//ビルダーの用意
	void createBuilder();
	//ファクトリーの用意
	void createFactory();

	void setLoadUI();

public:

	static GameManager* GetInstance()
	{
		if (!gameManager)
		{
			gameManager = new GameManager();
		}

		return gameManager;
	}

	//ゲームのフレームレートの設定やステージの読み込みを行う
	void initGame();

	//シーンの作成
	void createScene();

	//メインループ
	void mainGameLoop();
	//ステージから出る
	void exitScene();
	//ゲーム全体の終了処理
	void FinishGame();
};