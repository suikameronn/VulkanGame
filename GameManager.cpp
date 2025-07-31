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
	pipelineFactory = std::make_shared<PipelineFactory>(vulkanCore, pipelineLayoutFactory, shaderFactory
		, pipelineBuilder, renderPassFactory);

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
}

//シーンの作成
void GameManager::createScene()
{
	size_t entity1 = ecsManager->GenerateEntity();

	ecsManager->AddComponent<TransformComp>(entity1);
	GltfModelComp* comp = ecsManager->AddComponent<GltfModelComp>(entity1);
	ecsManager->AddComponent<GltfModelAnimComp>(entity1);
	ecsManager->AddComponent<MeshRendererComp>(entity1);

	comp->filePath = "models/PlayerModel.glb";
}

void GameManager::mainGameLoop()//メインゲームループ
{
    start = std::chrono::system_clock::now();//フレーム時間を計測

	OnStart();

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
	ecsManager->RunFunction<GltfModelComp>
		(
			{
				[&](GltfModelComp& comp)
				{
					comp.modelID = modelFactory->Load(comp.filePath);
				}
			}
		);

	ecsManager->RunFunction<GltfModelComp, MeshRendererComp>
		(
			{
				[&](GltfModelComp& modelComp,MeshRendererComp& rendererComp)
				{
					const std::shared_ptr<GltfModel> gltfModel =
						modelFactory->GetModel(modelComp.modelID);

					rendererComp.modelMatBuffer =
						bufferFactory->Create(sizeof(glm::mat4),BufferUsage::UNIFORM,BufferTransferType::NONE);

					rendererComp.animMatBuffer.resize(gltfModel->nodes.size());
					for (auto& buffer : rendererComp.animMatBuffer)
					{
						buffer = bufferFactory->Create(sizeof(glm::mat4)
							, BufferUsage::UNIFORM, BufferTransferType::NONE);
					}

					const std::shared_ptr<DescriptorSetLayout> layout =
						descriptorSetLayoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT);

					rendererComp.modelMatDesc =
						descriptorSetFactory->Create(
							descriptorSetBuilder->withBindingBuffer(0)
							.withBuffer(rendererComp.modelMatBuffer)
							.withDescriptorSetCount(1)
							.withDescriptorSetLayout(layout)
							.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
							.withRange(sizeof(glm::mat4))
							.Build()
						);

					rendererComp.animMatDesc.resize(gltfModel->nodes.size());
					for (int i = 0; i < rendererComp.animMatDesc.size(); i++)
					{
						rendererComp.animMatDesc[i] =
							descriptorSetFactory->Create(
								descriptorSetBuilder->withBindingBuffer(0)
								.withBuffer(rendererComp.animMatBuffer[i])
								.withDescriptorSetCount(1)
								.withDescriptorSetLayout(layout)
								.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
								.withRange(sizeof(glm::mat4))
								.Build()
							);
					}

				}
			}
		);

	ecsManager->RunFunction<PointLightComp>
		(
			{
			[&](PointLightComp& comp)
			{
				comp.uniformBuffer =
					bufferFactory->Create(sizeof(comp.uniform),BufferUsage::UNIFORM,BufferTransferType::NONE);

				comp.uniformDescriptorSet = descriptorSetFactory->Create
				(
					descriptorSetFactory->getBuilder()
					->initProperty()
					.withBindingBuffer(0)
					.withBuffer(comp.uniformBuffer)
					.withDescriptorSetCount(1)
					.withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
					.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.withRange(sizeof(comp.uniform))
					.addBufferInfo()
					.Build()
				);

				const VkExtent2D extent = swapChain->getSwapChainExtent();

				comp.shadowMap = textureFactory->ImageViewCreate
				(
					textureBuilder->initProperty()
					.withWidthHeight(extent.width, extent.height)
					.withViewType(VK_IMAGE_VIEW_TYPE_2D)
					.withViewAccess(VK_IMAGE_ASPECT_DEPTH_BIT)
					.withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
					.withTiling(VK_IMAGE_TILING_OPTIMAL)
					.withNumSamples(VK_SAMPLE_COUNT_1_BIT)
					.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
					.withLayerCount(1)
					.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
					.withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
					.withFormat(VK_FORMAT_D16_UNORM)
					.Build()
				);
			}
			}
		);
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

	{
		std::shared_ptr<CommandBuffer> commandBuffer = bufferFactory->CommandBufferCreate();
		std::shared_ptr<FrameBuffer> frameBuffer = swapChain->getCurrentFrameBuffer();

		bufferFactory->beginCommandBuffer(commandBuffer);

		const RenderProperty property = render->initProperty()
			.withRenderPass(renderPassFactory->Create(RenderPassPattern::PBR))
			.withFrameBuffer(frameBuffer)
			.withCommandBuffer(commandBuffer)
			.withRenderArea(swapChain->getSwapChainExtent().width, swapChain->getSwapChainExtent().height)
			.withClearColor({ 0.0, 0.0, 0.0, 1.0 })
			.withClearDepth(1.0f)
			.withClearStencil(0)
			.Build();

		std::function<void(GltfModelComp&, MeshRendererComp&)> renderModel =
			[&](GltfModelComp& gltfComp, MeshRendererComp& meshRendererComp)
			{
				std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(gltfComp.modelID);
				if (!gltfModel)
				{
					throw std::runtime_error("GameManager::Render::gltfModel is nullptr");
				}

				vkCmdBindPipeline(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineFactory->Create(PipelinePattern::PBR)->pipeline);

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)swapChain->getSwapChainExtent().width;
				viewport.height = (float)swapChain->getSwapChainExtent().height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(commandBuffer->commandBuffer, 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = swapChain->getSwapChainExtent();
				vkCmdSetScissor(commandBuffer->commandBuffer, 0, 1, &scissor);

				std::array<VkDescriptorSet, 1> descriptorSets;

				VkDeviceSize offsets[] = { 0 };

				for (const auto& node : gltfModel->nodes)
				{
					const Mesh& mesh = node.mesh;

					if (mesh.vertices.size() != 0)
					{
						vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

						vkCmdBindIndexBuffer(commandBuffer->commandBuffer, gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

						for (const auto& primitive : mesh.primitives)
						{
							std::shared_ptr<Material> material = gltfModel->materials[primitive.materialIndex];

							descriptorSets[0] = meshRendererComp.modelMatDesc->descriptorSet;
							//descriptorSets[1] = meshRendererComp.animMatDesc[primitive.primitiveIndex]->descriptorSet;

							vkCmdBindDescriptorSets(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayoutFactory->Create(PipelineLayoutPattern::PBR)->pLayout, 0
								, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

							vkCmdDrawIndexed(commandBuffer->commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
						}
					}
				}
			};

		render->RenderStart(property);

		ecsManager->RunFunction<GltfModelComp, MeshRendererComp>(renderModel);

		render->RenderEnd(property);

		bufferFactory->endCommandBuffer(commandBuffer);

		swapChain->flipSwapChainImage(commandBuffer);
	}

	//リソースの破棄
	descriptorSetLayoutFactory->resourceDestruct();
	bufferFactory->resourceDestruct();
	frameBufferFactory->resourceDestruct();
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