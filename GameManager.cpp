#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//ゲーム全体の初期化処理
{
	vulkanCore = std::make_shared<VulkanCore>(window);

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//指定したfpsから、一フレームにかけられる時間を計算する

    bool load = false;

	ecsManager = std::make_shared<ECSManager>();

	createInstance();//インスタンスの作成

	createScene();//シーンの作成

    mainGameLoop();//ゲームのメインループを開始
}

//インスタンスを作成する
void GameManager::createInstance()
{
    createBuilder();//ビルダーの用意
    createFactory();//ファクトリーの用意

	VkDevice& device = vulkanCore->getLogicDevice();

	pipelineBuilder = std::make_shared<PipelineBuilder>(device, shaderFactory);

	textureBuilder = std::make_shared<TextureBuilder>(vulkanCore, bufferFactory);
	textureFactory = std::make_shared<TextureFactory>(device, textureBuilder);

    materialBuilder = std::make_shared<MaterialBuilder>(bufferFactory, descriptorSetLayoutFactory
        , descriptorSetFactory, textureFactory);

	modelFactory = std::make_shared<GltfModelFactory>(materialBuilder, textureFactory
		, bufferFactory, descriptorSetLayoutFactory, descriptorSetFactory);

    swapChain = std::make_shared<SwapChain>(vulkanCore, textureFactory, renderPassFactory, frameBufferFactory);

	render = std::make_shared<Render>(device);
}

//ビルダーの用意
void GameManager::createBuilder()
{
	VkPhysicalDevice physicalDevice = vulkanCore->getPhysicalDevice();
	VkDevice device = vulkanCore->getLogicDevice();

	descriptorSetBuilder = std::make_shared<DescriptorSetBuilder>(device);
	frameBufferBuilder = std::make_shared<FrameBufferBuilder>(device);
	bufferBuilder = std::make_shared<GpuBufferBuilder>(physicalDevice, device);
	descriptorSetLayoutBuilder = std::make_shared<DescriptorSetLayoutBuilder>();
	pipelineLayoutBuilder = std::make_shared<PipelineLayoutBuilder>();
	renderPassBuilder = std::make_shared<RenderPassBuilder>(device);
}

//ファクトリーの用意
void GameManager::createFactory()
{
	VkDevice device = vulkanCore->getLogicDevice();

	frameBufferFactory = std::shared_ptr<FrameBufferFactory>(new FrameBufferFactory(device, frameBufferBuilder));

	descriptorSetLayoutFactory = std::make_shared<DescriptorSetLayoutFactory>(device, descriptorSetLayoutBuilder);
	descriptorSetFactory = std::make_shared<DescriptorSetFactory>(device, descriptorSetBuilder, descriptorSetLayoutFactory);
	bufferFactory = std::make_shared<GpuBufferFactory>(vulkanCore, bufferBuilder);
	pipelineLayoutFactory = std::make_shared<PipelineLayoutFactory>(device, pipelineLayoutBuilder, descriptorSetLayoutFactory);
	renderPassFactory = std::make_shared<RenderPassFactory>(vulkanCore, renderPassBuilder);
	shaderFactory = std::make_shared<ShaderFactory>(device);
	pipelineFactory = std::make_shared<PipelineFactory>(device, pipelineLayoutFactory, shaderFactory, pipelineBuilder, renderPassFactory);
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

		OnUpdate();//コンポーネントの更新処理

		OnLateUpdate();//更新処理後の処理

		Rendering();//オブジェクトのレンダリング

		OnFrameEnd();//フレーム終了時の処理

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

//コンポーネントの初回処理
void GameManager::OnStart()
{
	//ecsManager->RunFunction<
}

//コンポーネントの更新処理
void GameManager::OnUpdate()
{

}

//更新処理後の処理
void GameManager::OnLateUpdate()
{

}

//オブジェクトのレンダリング
void GameManager::Rendering()
{
	//ecsManager->RunFunction<

	//リソースの破棄
	descriptorSetLayoutFactory->resourceDestruct();
	bufferFactory->resourceDestruct();
	//frameBufferFactory->resourceDestruct();
	pipelineLayoutFactory->resourceDestruct();
	pipelineFactory->resourceDestruct();
	renderPassFactory->resourceDestruct();
	shaderFactory->resourceDestruct();
	textureFactory->resourceDestruct();
}

//フレーム終了時の処理
void GameManager::OnFrameEnd()
{

}

//シーンを終了させる処理
void GameManager::exitScene()
{
    //ゲームを終了させる
    FinishGame();
}

//ゲーム全体の終了処理
void GameManager::FinishGame()
{

}