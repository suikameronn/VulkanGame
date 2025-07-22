#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//ゲーム全体の初期化処理
{
	vulkanCore = std::make_shared<VulkanCore>();

	createBuilder();//ビルダーの用意
	createFactory();//ファクトリーの用意

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//指定したfpsから、一フレームにかけられる時間を計算する

    bool load = false;

	ecsManager = std::make_shared<ECSManager>();

	createScene();//シーンの作成

    mainGameLoop();//ゲームのメインループを開始
}

//ビルダーの用意
void GameManager::createBuilder()
{
	descriptorSetBuilder = std::make_shared<DescriptorSetBuilder>(vulkanCore);
	frameBufferBuilder = std::make_shared<FrameBufferBuilder>(vulkanCore);
	bufferBuilder = std::make_shared<GpuBufferBuilder>(vulkanCore);
	descriptorSetLayoutBuilder = std::make_shared<DescriptorSetLayoutBuilder>(vulkanCore);
	pipelineLayoutBuilder = std::make_shared<PipelineLayoutBuilder>(vulkanCore);
	pipelineBuilder = std::make_shared<PipelineBuilder>(vulkanCore);
	renderPassBuilder = std::make_shared<RenderPassBuilder>(vulkanCore);
}

//ファクトリーの用意
void GameManager::createFactory()
{
	descriptorSetFactory = std::make_shared<DescriptorSetFactory>(vulkanCore, descriptorSetBuilder);
	frameBufferFactory = std::make_shared<FrameBufferFactory>(vulkanCore, frameBufferBuilder);
	bufferFactory = std::make_shared<GpuBufferFactory>(vulkanCore, bufferBuilder);
	descriptorSetLayoutFactory = std::make_shared<DescriptorSetLayoutFactory>(vulkanCore, descriptorSetLayoutBuilder);
	pipelineLayoutFactory = std::make_shared<PipelineLayoutFactory>(vulkanCore, pipelineLayoutBuilder);
	pipelineFactory = std::make_shared<PipelineFactory>(vulkanCore, pipelineBuilder);
	renderPassFactory = std::make_shared<RenderPassFactory>(vulkanCore, renderPassBuilder);
}

//シーンの作成
void GameManager::createScene()
{

}

void GameManager::mainGameLoop()//メインゲームループ
{
    start = std::chrono::system_clock::now();//フレーム時間を計測

    while (true)
    {

        if (vulkanCore->isShouldCloseWindow())//シーンを終了するかどうか
        {
            break;
        }

        end = std::chrono::system_clock::now();//フレーム終了時間を計測
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();//フレーム間時間を計算する
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//fpsを制限する
        }
        else
        {
            std::cout << "fps down" << std::endl;
        }
        start = std::chrono::system_clock::now();
    }

    exitScene();
}

//シーンを狩猟させる処理
void GameManager::exitScene()
{
    //ゲームを終了させる
    FinishGame();
}

//ゲーム全体の終了処理
void GameManager::FinishGame()
{

}