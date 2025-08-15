#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//ゲーム全体の初期化処理
{
	vulkanCore = std::make_shared<VulkanCore>(window);

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//指定したfpsから、一フレームにかけられる時間を計算する

    bool load = false;

	ecsManager = std::make_shared<ECSManager>();

	createInstance();//インスタンスの作成

	sceneLight = std::make_shared<SceneLight>();

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

	skydomeBuilder = std::make_shared<SkyDomeBuilder>(vulkanCore, bufferFactory, textureFactory, frameBufferFactory
		, pipelineLayoutFactory, pipelineFactory, renderPassFactory
		, descriptorSetLayoutFactory, descriptorSetFactory, render, modelFactory);

	skydomeFactory = std::make_shared<SkyDomeFactory>(skydomeBuilder);
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

	ecsManager->AddComponent<TransformComp>(entity1)->scale = glm::vec3(10.0f);

	GltfModelComp* comp = ecsManager->AddComponent<GltfModelComp>(entity1);

	GltfModelAnimComp* animComp = ecsManager->AddComponent<GltfModelAnimComp>(entity1);
	animComp->animationName = "Idle";

	ecsManager->AddComponent<MeshRendererComp>(entity1);

	comp->filePath = "models/robot.glb";

	size_t entity2 = ecsManager->GenerateEntity();
	ecsManager->AddComponent<DirectionLightComp>(entity2);
	ecsManager->AddComponent<TransformComp>(entity2)->position = glm::vec3(10.0f);

	size_t entity3 = ecsManager->GenerateEntity();
	SkyDomeComp* skydomeComp = ecsManager->AddComponent<SkyDomeComp>(entity3);

	size_t entity4 = ecsManager->GenerateEntity();
	CameraComp* cameraComp = ecsManager->AddComponent<CameraComp>(entity4);
	cameraComp->aspect = 900.0f / 600.0f;
	cameraComp->viewAngle = 45.0f;
	cameraComp->matrices.position = glm::vec3(-10.0f);
	cameraComp->matrices.view = glm::lookAt(cameraComp->matrices.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	cameraComp->matrices.proj = glm::perspective(cameraComp->viewAngle, cameraComp->aspect, 0.1f, 1000.0f);

	size_t entity5 = ecsManager->GenerateEntity();
	DirectionLightComp* dLight = ecsManager->AddComponent<DirectionLightComp>(entity5);
	dLight->color = glm::vec4(1.0f);
	dLight->direction = glm::vec3(1.0f);
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
            //std::cout << "fps down" << std::endl;
        }
        start = std::chrono::system_clock::now();

		glfwPollEvents();
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

	ecsManager->RunFunction<GltfModelComp, GltfModelAnimComp>
		(
			{
				[&](GltfModelComp& modelComp,GltfModelAnimComp& animComp)
				{
					const std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(modelComp.modelID);

					animComp.nodeTransform.init();
					animComp.nodeTransform.setNodeCount(gltfModel->nodes.size());

					animComp.startTime = clock();
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

					{
						//モデル行列用のバッファを作る

						rendererComp.modelMatBuffer =
							bufferFactory->Create(sizeof(ModelMat), BufferUsage::UNIFORM, BufferTransferType::NONE);
					}

					{
						//ノードごとの行列など用のバッファを作る
						rendererComp.nodeAnim.resize(gltfModel->nodes.size());
						rendererComp.nodeAnimBuffer.resize(gltfModel->nodes.size());

						for (int i = 0; i < gltfModel->nodes.size(); i++)
						{
							if (gltfModel->nodes[i].mesh.vertices.size() != 0)
							{
								rendererComp.nodeAnimBuffer[i] = bufferFactory->Create(sizeof(NodeAnimMat)
									, BufferUsage::UNIFORM, BufferTransferType::NONE);
							}
						}
					}

					{
						//ノード全体のアニメーション行列用のバッファを作る

						rendererComp.boneMatBuffer = bufferFactory->Create(sizeof(BoneMat)
							, BufferUsage::UNIFORM, BufferTransferType::NONE);
					}

					const std::shared_ptr<DescriptorSetLayout> layout =
						descriptorSetLayoutFactory->Create(LayoutPattern::MODELANIMMAT);

					{
						rendererComp.modelAnimDesc.resize(gltfModel->nodes.size());
						for (int i = 0; i < gltfModel->nodes.size(); i++)
						{
							if (gltfModel->nodes[i].mesh.vertices.size() != 0)
							{
								rendererComp.modelAnimDesc[i] =
									descriptorSetFactory->Create
									(
										descriptorSetBuilder->initProperty()
										.withBindingBuffer(0)
										.withBuffer(rendererComp.modelMatBuffer)
										.withDescriptorSetCount(1)
										.withDescriptorSetLayout(layout)
										.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
										.withRange(sizeof(ModelMat))
										.addBufferInfo()
										.withBindingBuffer(1)
										.withBuffer(rendererComp.nodeAnimBuffer[i])
										.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
										.withRange(sizeof(NodeAnimMat))
										.addBufferInfo()
										.withBindingBuffer(2)
										.withBuffer(rendererComp.boneMatBuffer)
										.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
										.withRange(sizeof(BoneMat))
										.addBufferInfo()
										.Build()
									);
							}
						}
					}
				}
			}
		);

	{
		{
			int pointLightCount = static_cast<int>(ecsManager->GetEntitiesWithComponents<PointLightComp>().size());

			//ライトの数だけシャドウマップのテクスチャのレイヤー数を作成する

			const VkExtent2D extent = swapChain->getSwapChainExtent();

			//ライトコンポーネントが無くても、少なくとも一つはあるものとして
			//リソースを作成する
			pointLightCount = (pointLightCount > 0) ? pointLightCount : 1;

			//ポイントライト用のユニフォームバッファを作る
			//同一のライトは、すべて一つのバッファに配列として格納される
			sceneLight->uniformBuffer[0] = bufferFactory->Create
			(
				sizeof(PointLightUniform),
				&sceneLight->pointUniform,
				BufferUsage::UNIFORM,
				BufferTransferType::NONE
			);

			//ユニフォームバッファのVkDescriptorSetを作成する
			sceneLight->uniformDescriptorSet[0] = descriptorSetFactory->Create
			(
				descriptorSetBuilder->initProperty()
				.withDescriptorSetCount(1)
				.withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
				.withBindingBuffer(0)
				.withBuffer(sceneLight->uniformBuffer[0])
				.withRange(sizeof(PointLightUniform))
				.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.addBufferInfo()
				.Build()
			);

			//ポイントライトの用の複数のレイヤーを持つシャドウマップ用のテクスチャを作成する
			sceneLight->shadowMap[0] = textureFactory->Create
			(
				textureBuilder->initProperty()
				.withWidthHeight(extent.width, extent.height, 1)
				.withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				.withTiling(VK_IMAGE_TILING_OPTIMAL)
				.withNumSamples(VK_SAMPLE_COUNT_1_BIT)
				.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				.withLayerCount(pointLightCount)
				.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.withFormat(VK_FORMAT_D16_UNORM)
				.withViewAccess(VK_IMAGE_ASPECT_DEPTH_BIT)
				.withViewType(VK_IMAGE_VIEW_TYPE_2D_ARRAY)
				.withTargetLayer(0, pointLightCount)
				.addView()
				.withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
				.withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
				.withMagFilter(VK_FILTER_LINEAR)
				.withMinFilter(VK_FILTER_LINEAR)
				.Build()
			);

			//各ビューからフレームバッファを作成する
			//テクスチャの各レイヤーに書き込めるようになる
			sceneLight->frameBuffer[0].resize(pointLightCount);
			for (int i = 0; i < pointLightCount; i++)
			{
				sceneLight->frameBuffer[0][i] = frameBufferFactory->Create
				(
					frameBufferFactory->getBuilder()->initProperty()
					.withLayerCount(1)
					.withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
					.withWidthHeight(extent.width, extent.height)
					.addViewAttachment(sceneLight->shadowMap[0], i, 1)
					.Build()
				);
			}
		}

		{
			int directionLightCount = static_cast<int>(ecsManager->GetEntitiesWithComponents<DirectionLightComp>().size());

			//ライトコンポーネントが無くても、少なくとも一つはあるものとして
			//リソースを作成する
			directionLightCount = (directionLightCount > 0) ? directionLightCount : 1;

			const VkExtent2D extent = swapChain->getSwapChainExtent();

			//ディレクションライト用のユニフォームバッファを作る
			//同一のライトは、すべて一つのバッファに配列として格納される
			sceneLight->uniformBuffer[1] = bufferFactory->Create
			(
				sizeof(DirectionLightUniform),
				&sceneLight->dirUniform,
				BufferUsage::UNIFORM,
				BufferTransferType::NONE
			);

			//ユニフォームバッファのVkDescriptorSetを作成する
			sceneLight->uniformDescriptorSet[1] = descriptorSetFactory->Create
			(
				descriptorSetBuilder->initProperty()
				.withDescriptorSetCount(1)
				.withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
				.withBindingBuffer(0)
				.withBuffer(sceneLight->uniformBuffer[1])
				.withRange(sizeof(DirectionLightUniform))
				.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.addBufferInfo()
				.Build()
			);

			//ディレクションライトの用の複数のレイヤーを持つシャドウマップ用のテクスチャを作成する
			textureBuilder->initProperty()
				.withWidthHeight(extent.width, extent.height, 1)
				.withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				.withTiling(VK_IMAGE_TILING_OPTIMAL)
				.withNumSamples(VK_SAMPLE_COUNT_1_BIT)
				.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				.withLayerCount(directionLightCount)
				.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.withFormat(VK_FORMAT_D16_UNORM);

			for (int i = 0; i < directionLightCount; i++)
			{
				textureBuilder->withViewAccess(VK_IMAGE_ASPECT_DEPTH_BIT)
					.withViewType(VK_IMAGE_VIEW_TYPE_2D_ARRAY)
					.withTargetLayer(i, 1)
					.addView();
			}

			textureBuilder->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
				.withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
				.withMagFilter(VK_FILTER_LINEAR)
				.withMinFilter(VK_FILTER_LINEAR)
				.Build();

			sceneLight->shadowMap[1] = textureFactory->Create
			(
				textureBuilder->Build()
			);

			//各ビューからフレームバッファを作成する
			//テクスチャの各レイヤーに書き込めるようになる
			sceneLight->frameBuffer[1].resize(directionLightCount);
			for (int i = 0; i < directionLightCount; i++)
			{
				sceneLight->frameBuffer[1][i] = frameBufferFactory->Create
				(
					frameBufferFactory->getBuilder()->initProperty()
					.withLayerCount(1)
					.withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
					.withWidthHeight(extent.width, extent.height)
					.addViewAttachment(sceneLight->shadowMap[1], i, 1)
					.Build()
				);
			}
		}

		//ライトをまとめて一つのVkDescriptorSetを管理する
		sceneLight->texDescriptorSet = descriptorSetFactory->Create
		(
			descriptorSetBuilder->initProperty()
			.withDescriptorSetCount(1)
			.withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::SHADOWMAP))
			.withBindingBuffer(0)
			.withBuffer(sceneLight->uniformBuffer[0])
			.withRange(sizeof(PointLightUniform))
			.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			.addBufferInfo()
			.withBindingBuffer(1)
			.withBuffer(sceneLight->uniformBuffer[1])
			.withRange(sizeof(DirectionLightUniform))
			.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			.addBufferInfo()
			.withBindingImage(2)
			.withTexture(sceneLight->shadowMap[0])
			.withImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			.withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			.addImageInfo()
			.withBindingImage(3)
			.withTexture(sceneLight->shadowMap[1])
			.withImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			.withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			.addImageInfo()
			.Build()
		);
	}

	{
		//ライトコンポーネントから各ライトのパラメータを取り出し
		//上で作成したバッファに入れていく

		ecsManager->RunFunction<PointLightComp>
			(
				{
					[&](PointLightComp& comp)
					{
						comp.index = sceneLight->getPointLightIndex();

						sceneLight->pointUniform.position[comp.index] = comp.position;
						sceneLight->pointUniform.color[comp.index] = comp.color;
					}
				}
			);

		ecsManager->RunFunction<DirectionLightComp>
			(
				{
					[&](DirectionLightComp& comp)
					{
						comp.index = sceneLight->getDirectionLightIndex();

						sceneLight->dirUniform.direction[comp.index] = comp.direction;
						sceneLight->dirUniform.color[comp.index] = comp.color;
					}
				}
			);
	}

	ecsManager->RunFunction<SkyDomeComp>
		(
			{
				[&](SkyDomeComp& comp)
				{
					comp.ID = skydomeFactory->Create
					(
						skydomeBuilder->initProperty()
						.withImagePath("textures\\hdri_map.hdr")
						.Build()
					);
				}
			}
		);

	ecsManager->RunFunction<CameraComp>
		(
			{
				[&](CameraComp& comp)
				{
					comp.uniform = bufferFactory->Create
					(
						sizeof(CameraUniform),
						&comp.matrices,
						BufferUsage::UNIFORM,
						BufferTransferType::NONE
					);

					comp.descriptorSet = descriptorSetFactory->Create
						(
							descriptorSetBuilder->initProperty()
							.withDescriptorSetCount(1)
							.withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::CAMERA))
							.withBindingBuffer(0)
							.withBuffer(comp.uniform)
							.withRange(sizeof(CameraUniform))
							.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
							.addBufferInfo()
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
	ecsManager->RunFunction<TargetEntityComp, TransformComp>
		(
			{
				[&](TargetEntityComp& targetComp,TransformComp& transComp)
				{
					TransformComp* target = ecsManager->GetComponent<TransformComp>(targetComp.targetEntity);

					transComp.position = target->position + targetComp.offset;
				}
			}
		);

	ecsManager->RunFunction<TargetEntityComp, TransformComp, CameraComp>
		(
			{
				[&](TargetEntityComp& targetComp,TransformComp& transComp,CameraComp& cameraComp)
				{
					TransformComp* target = ecsManager->GetComponent<TransformComp>(targetComp.targetEntity);

					cameraComp.matrices.view = glm::lookAt(transComp.position, target->position, glm::vec3(0.0f, 1.0f, 0.0f));
					cameraComp.matrices.proj = glm::perspective(cameraComp.viewAngle, cameraComp.aspect, cameraComp.zNear, cameraComp.zFar);
				}
			}
		);

	ecsManager->RunFunction<DirectionLightComp, TransformComp>
		(
			{
				[&](DirectionLightComp& dirLightComp,TransformComp& transComp)
				{
					sceneLight->dirUniform.lightCount
						= static_cast<uint32_t>(ecsManager->GetEntitiesWithComponents<DirectionLightComp>().size());

					sceneLight->dirUniform.color[dirLightComp.index] = dirLightComp.color;

					sceneLight->dirUniform.direction[dirLightComp.index] = dirLightComp.direction;

					sceneLight->dirUniform.viewProj[dirLightComp.index] = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 0.1f, 1000.0f)
						* glm::lookAt(transComp.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				}
			}
		);

	ecsManager->RunFunction<TransformComp, MeshRendererComp>
		(
			{
			[&](TransformComp& transComp,MeshRendererComp& rendererComp)
			{
				glm::mat4 rotateMat = glm::mat4(1.0f);
				rotateMat *= glm::rotate(glm::radians(transComp.rotation[0]), glm::vec3(1, 0, 0));
				rotateMat *= glm::rotate(glm::radians(transComp.rotation[1]), glm::vec3(0, 1, 0));
				rotateMat *= glm::rotate(glm::radians(transComp.rotation[2]), glm::vec3(0, 0, 1));

				const glm::mat4 model = glm::translate(glm::mat4(1.0f),transComp.position)
					* rotateMat * glm::scale(glm::mat4(1.0f),transComp.scale);

				ModelMat modelMat{};
				modelMat.scale = transComp.scale;
				modelMat.matrix = model;

				bufferFactory->copyMemory(sizeof(ModelMat), &modelMat, rendererComp.modelMatBuffer);
			}
			}
		);

	ecsManager->RunFunction<GltfModelComp, GltfModelAnimComp, MeshRendererComp>
		(
			{
				[&](GltfModelComp& modelComp, GltfModelAnimComp& animComp, MeshRendererComp& rendererComp)
				{
					const std::shared_ptr<GltfModel> gltfModel =
							modelFactory->GetModel(modelComp.modelID);

					std::vector<std::array<glm::mat4, 128>> jointMat(1);

					float deltaTime = animComp.startTime;
					if (deltaTime > gltfModel->animations[animComp.animationName].end)
					{
						deltaTime = gltfModel->animations[animComp.animationName].start;
						animComp.startTime = clock();
					}

					gltfModel->updateAnimation(deltaTime, animComp.animationName, animComp.nodeTransform, jointMat);

					bufferFactory->copyMemory(sizeof(BoneMat), &jointMat[0], rendererComp.boneMatBuffer);

					for (int i = 0; i < gltfModel->nodes.size(); i++)
					{
						const GltfNode& node = gltfModel->nodes[i];

						if (node.mesh.vertices.size() != 0)
						{
							rendererComp.nodeAnim[i].boneCount = node.getJointCount();
							rendererComp.nodeAnim[i].matrix = node.matrix;
							rendererComp.nodeAnim[i].nodeMatrix = animComp.nodeTransform.nodeTransform[node.offset];

							bufferFactory->copyMemory(sizeof(NodeAnimMat), &rendererComp.nodeAnim[i], rendererComp.nodeAnimBuffer[i]);
						}
					}
				}
			}
		);
}

//オブジェクトのレンダリング
void GameManager::Rendering()
{
	{
		//シャドウマップを作成する
		ecsManager->RunFunction<DirectionLightComp>
			(
				{
					[&](DirectionLightComp& comp)
					{
						std::shared_ptr<CommandBuffer> commandBuffer = bufferFactory->CommandBufferCreate();

						bufferFactory->beginCommandBuffer(commandBuffer);

						const VkExtent2D extent = swapChain->getSwapChainExtent();

						const RenderProperty renderProp = render->initProperty()
							.withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
							.withFrameBuffer(sceneLight->frameBuffer[1][comp.index])
							.withCommandBuffer(commandBuffer)
							.withRenderArea(extent.width, extent.height)
							.withClearDepth(1.0f)
							.withClearStencil(0)
							.Build();

						render->RenderStart(renderProp);

						ecsManager->RunFunction<GltfModelComp, MeshRendererComp>
							(
								{
									[&](GltfModelComp& modelComp,MeshRendererComp& rendererComp)
									{
										std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(modelComp.modelID);
										if (!gltfModel)
										{
											throw std::runtime_error("GameManager::Render::gltfModel is nullptr");
										}

										vkCmdBindPipeline(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
											pipelineFactory->Create(PipelinePattern::CALC_SHADOWMAP)->pipeline);

										vkCmdSetDepthBias(commandBuffer->commandBuffer, 1.25f, 0.0f, 1.75f);

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

										std::array<VkDescriptorSet, 2> descriptorSets;

										VkDeviceSize offsets[] = { 0 };

										for (int i = 0; i < gltfModel->nodes.size(); i++)
										{
											const Mesh& mesh = gltfModel->nodes[i].mesh;

											if (mesh.vertices.size() != 0)
											{
												vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

												vkCmdBindIndexBuffer(commandBuffer->commandBuffer, gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

												for (const auto& primitive : mesh.primitives)
												{
													std::shared_ptr<Material> material = gltfModel->materials[primitive.materialIndex];

													descriptorSets[0] = rendererComp.modelAnimDesc[i]->descriptorSet;
													descriptorSets[1] = sceneLight->uniformDescriptorSet[1]->descriptorSet;

													vkCmdBindDescriptorSets(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
														pipelineLayoutFactory->Create(PipelineLayoutPattern::CALC_SHADOWMAP)->pLayout, 0
														, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

													vkCmdDrawIndexed(commandBuffer->commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
												}
											}
										}
									}
								}
							);

						render->RenderEnd(renderProp);

						bufferFactory->endCommandBuffer(commandBuffer);

						VkPipelineStageFlags flag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

						VkSubmitInfo thirdSubmitInfo{};
						thirdSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
						thirdSubmitInfo.commandBufferCount = 1;
						thirdSubmitInfo.pCommandBuffers = &commandBuffer->commandBuffer;
						thirdSubmitInfo.pWaitDstStageMask = &flag;

						VkFenceCreateInfo fenceInfo{};
						fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
						fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

						VkFence fence;

						vkCreateFence(vulkanCore->getLogicDevice(), &fenceInfo, nullptr, &fence);
						vkResetFences(vulkanCore->getLogicDevice(), 1, &fence);

						if (vkQueueSubmit(vulkanCore->getGraphicsQueue(), 1, &thirdSubmitInfo, fence) != VK_SUCCESS)
						{
							throw std::runtime_error("failed to submit draw command buffer!");
						}

						vkWaitForFences(vulkanCore->getLogicDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

						vkDestroyFence(vulkanCore->getLogicDevice(), fence, nullptr);
					}
				}
			);
	}

	{
		std::shared_ptr<CommandBuffer> commandBuffer = bufferFactory->CommandBufferCreate();
		std::shared_ptr<FrameBuffer> frameBuffer = swapChain->getCurrentFrameBuffer();

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

				FragmentParam param{};
				param.alphaness = 1.0f;
				vkCmdPushConstants(commandBuffer->commandBuffer, pipelineLayoutFactory->Create(PipelineLayoutPattern::PBR)->pLayout
					, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(FragmentParam), &param);

				std::array<VkDescriptorSet, 5> descriptorSets;

				VkDeviceSize offsets[] = { 0 };

				for (int i = 0; i < gltfModel->nodes.size(); i++)
				{
					const Mesh& mesh = gltfModel->nodes[i].mesh;

					if (mesh.vertices.size() != 0)
					{
						vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

						vkCmdBindIndexBuffer(commandBuffer->commandBuffer, gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

						for (const auto& primitive : mesh.primitives)
						{
							std::shared_ptr<Material> material = gltfModel->materials[primitive.materialIndex];

							size_t cameraEntity = ecsManager->GetEntitiesWithComponents<CameraComp>()[0];
							size_t skydomeEntity = ecsManager->GetEntitiesWithComponents<SkyDomeComp>()[0];
							std::shared_ptr<SkyDome> skydome = skydomeFactory->GetSkyDome(ecsManager->GetComponent<SkyDomeComp>(skydomeEntity)->ID);

							descriptorSets[0] = meshRendererComp.modelAnimDesc[i]->descriptorSet;
							descriptorSets[1] = ecsManager->GetComponent<CameraComp>(cameraEntity)->descriptorSet->descriptorSet;
							descriptorSets[2] = sceneLight->texDescriptorSet->descriptorSet;
							descriptorSets[3] = gltfModel->materials[primitive.materialIndex]->getDescriptorSet()->descriptorSet;
							descriptorSets[4] = skydome->descriptorSet->descriptorSet;

							vkCmdBindDescriptorSets(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayoutFactory->Create(PipelineLayoutPattern::PBR)->pLayout, 0
								, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

							vkCmdDrawIndexed(commandBuffer->commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
						}
					}
				}
			};

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