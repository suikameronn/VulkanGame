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

	createRenderCommand();//レンダー用コマンドバッファの作成

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

	textureBuilder = std::make_shared<TextureBuilder>(vulkanCore, bufferFactory, commandBufferFactory);
	textureFactory = std::make_shared<TextureFactory>(device, textureBuilder);

    materialBuilder = std::make_shared<MaterialBuilder>(bufferFactory, descriptorSetLayoutFactory
        , descriptorSetFactory, textureFactory);

	modelFactory = std::make_shared<GltfModelFactory>(materialBuilder, textureFactory
		, bufferFactory, descriptorSetLayoutFactory, descriptorSetFactory);

	swapChain = std::make_shared<SwapChain>(vulkanCore, textureFactory, renderPassFactory, frameBufferFactory, commandBufferFactory);

	render = std::make_shared<Render>(device);

	skydomeBuilder = std::make_shared<SkyDomeBuilder>(vulkanCore, bufferFactory, textureFactory, frameBufferFactory
		, pipelineLayoutFactory, pipelineFactory, renderPassFactory
		, descriptorSetLayoutFactory, descriptorSetFactory, render, modelFactory
		, commandBufferFactory);

	skydomeFactory = std::make_shared<SkyDomeFactory>(skydomeBuilder);

	coliderFactory = std::make_shared<ColiderFactory>(modelFactory, bufferFactory, descriptorSetLayoutFactory, descriptorSetFactory);

	inputSystem = std::make_shared<InputSystem>();
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
	commandBufferFactory = std::make_shared<CommandBufferFactory>(vulkanCore);
}

//シーンの作成
void GameManager::createScene()
{

	size_t entity1 = ecsManager->GenerateEntity();
	{
		TransformComp* transComp = ecsManager->AddComponent<TransformComp>(entity1);
		transComp->scale = glm::vec3(6.0f);
		transComp->rotation = glm::vec3(0.0f, 180.0f, 0.0f);

		GltfModelComp* comp = ecsManager->AddComponent<GltfModelComp>(entity1);

		GltfModelAnimComp* animComp = ecsManager->AddComponent<GltfModelAnimComp>(entity1);
		animComp->animationName = "Idle_gunMiddle";

		ecsManager->AddComponent<MeshRendererComp>(entity1);

		comp->filePath = "models/robot.glb";

		ecsManager->AddComponent<ColiderComp>(entity1);

		PhysicComp* physic = ecsManager->AddComponent<PhysicComp>(entity1);
		//physic->gravity = glm::vec3(0.0f, 0.98f, 0.0f);

		InputComp* input = ecsManager->AddComponent<InputComp>(entity1);
	}

	{
		size_t entity2 = ecsManager->GenerateEntity();
		DirectionLightComp* dLight = ecsManager->AddComponent<DirectionLightComp>(entity2);
		dLight->position = glm::vec3(100.0f, 100.0f, 100.0f);
		dLight->color = glm::vec4(1.0f);
		dLight->direction = glm::vec3(-100.0f, -100.0f, -100.0f);
		ecsManager->AddComponent<TransformComp>(entity2)->position = glm::vec3(10.0f);
	}

	{
		size_t entity3 = ecsManager->GenerateEntity();
		SkyDomeComp* skydomeComp = ecsManager->AddComponent<SkyDomeComp>(entity3);
	}

	{
		size_t entity4 = ecsManager->GenerateEntity();
		CameraComp* cameraComp = ecsManager->AddComponent<CameraComp>(entity4);
		cameraComp->aspect = 900.0f / 600.0f;
		cameraComp->viewAngle = 45.0f;
		cameraComp->matrices.position = glm::vec3(0.0f, -13.0f, -25.0f);
		cameraComp->matrices.view = glm::lookAt(cameraComp->matrices.position, glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		cameraComp->matrices.proj = glm::perspective(cameraComp->viewAngle, cameraComp->aspect, 0.1f, 1000.0f);

		TargetEntityComp* target = ecsManager->AddComponent<TargetEntityComp>(entity4);
		target->targetEntity = entity1;
		target->offset = glm::vec3(0.0f, -13.0f, -25.0f);
	}

	{
		size_t ground = ecsManager->GenerateEntity();

		TransformComp* transComp = ecsManager->AddComponent<TransformComp>(ground);
		transComp->position = glm::vec3(0.0f, 0.0f, -5.0f);
		transComp->scale = glm::vec3(100.0f,100.0f,100.0f);
		transComp->rotation = glm::vec3(0.0f, 180.0f, 0.0f);

		ColiderComp* colider = ecsManager->AddComponent<ColiderComp>(ground);
		colider->type = ColliderType::Box;

		GltfModelComp* gltfModel = ecsManager->AddComponent<GltfModelComp>(ground);
		gltfModel->filePath = "models/Ground.glb";

		ecsManager->AddComponent<MeshRendererComp>(ground);
	}
	/*size_t entity5 = ecsManager->GenerateEntity();
	DirectionLightComp* dLight2 = ecsManager->AddComponent<DirectionLightComp>(entity5);
	dLight2->position = glm::vec3(200.0f, 200.0f, 200.0f);
	dLight2->color = glm::vec4(1.0f);
	dLight2->direction = glm::vec3(-200.0f, -200.0f, -200.0f);
	*/
}

//レンダー用コマンドバッファの作成
void GameManager::createRenderCommand()
{
	renderCommand.resize(2);

	//あらかじめVkCommandBufferとVkFenceを作成しておく
	for (auto& command : renderCommand)
	{
		command = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandBufferFactory);

		command->setCommandBufffer(commandBufferFactory->createCommandBuffer(1))
			->setFence(commandBufferFactory->createFence())
			->setSemaphore(commandBufferFactory->createSemaphore(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	}
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
            //std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//fpsを制限する
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
										->withBindingBuffer(0)
										->withBuffer(rendererComp.modelMatBuffer)
										->withDescriptorSetCount(1)
										->withDescriptorSetLayout(layout)
										->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
										->withRange(sizeof(ModelMat))
										->addBufferInfo()
										->withBindingBuffer(1)
										->withBuffer(rendererComp.nodeAnimBuffer[i])
										->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
										->withRange(sizeof(NodeAnimMat))
										->addBufferInfo()
										->withBindingBuffer(2)
										->withBuffer(rendererComp.boneMatBuffer)
										->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
										->withRange(sizeof(BoneMat))
										->addBufferInfo()
										->Build()
									);
							}
						}
					}
				}
			}
		);

	ecsManager->RunFunction<GltfModelComp, ColiderComp>
		(
			{
				[&](GltfModelComp& gltfModel,ColiderComp& colider)
				{
					colider.ID = coliderFactory->Create(gltfModel.modelID);

					std::unique_ptr<Colider>& coliderPtr = coliderFactory->GetColider(colider.ID);

					coliderPtr->createBuffer();
					coliderPtr->createDescriptorSet();
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
				->withDescriptorSetCount(1)
				->withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
				->withBindingBuffer(0)
				->withBuffer(sceneLight->uniformBuffer[0])
				->withRange(sizeof(PointLightUniform))
				->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				->addBufferInfo()
				->Build()
			);

			//ポイントライトの用の複数のレイヤーを持つシャドウマップ用のテクスチャを作成する
			sceneLight->shadowMap[0] = textureFactory->Create
			(
				textureBuilder->initProperty()
				->withWidthHeight(extent.width, extent.height, 1)
				->withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				->withTiling(VK_IMAGE_TILING_OPTIMAL)
				->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
				->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				->withLayerCount(pointLightCount)
				->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				->withFormat(VK_FORMAT_D16_UNORM)
				->withViewAccess(VK_IMAGE_ASPECT_DEPTH_BIT)
				->withViewType(VK_IMAGE_VIEW_TYPE_2D_ARRAY)
				->withTargetLayer(0, pointLightCount)
				->addView()
				->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
				->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
				->withMagFilter(VK_FILTER_LINEAR)
				->withMinFilter(VK_FILTER_LINEAR)
				->Build()
			);

			//各ビューからフレームバッファを作成する
			//テクスチャの各レイヤーに書き込めるようになる
			sceneLight->frameBuffer[0].resize(pointLightCount);
			for (int i = 0; i < pointLightCount; i++)
			{
				sceneLight->frameBuffer[0][i] = frameBufferFactory->Create
				(
					frameBufferFactory->getBuilder()->initProperty()
					->withLayerCount(1)
					->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
					->withWidthHeight(extent.width, extent.height)
					->addViewAttachment(sceneLight->shadowMap[0], i, 1)
					->Build()
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
				->withDescriptorSetCount(1)
				->withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
				->withBindingBuffer(0)
				->withBuffer(sceneLight->uniformBuffer[1])
				->withRange(sizeof(DirectionLightUniform))
				->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				->addBufferInfo()
				->Build()
			);

			//ディレクションライトの用の複数のレイヤーを持つシャドウマップ用のテクスチャを作成する
			textureBuilder->initProperty()
				->withWidthHeight(extent.width, extent.height, 1)
				->withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				->withTiling(VK_IMAGE_TILING_OPTIMAL)
				->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
				->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				->withLayerCount(directionLightCount)
				->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				->withFormat(VK_FORMAT_D16_UNORM);

			for (int i = 0; i < directionLightCount; i++)
			{
				textureBuilder->withViewAccess(VK_IMAGE_ASPECT_DEPTH_BIT)
					->withViewType(VK_IMAGE_VIEW_TYPE_2D)
					->withTargetLayer(i, 1)
					->addView();
			}

			textureBuilder->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
				->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
				->withMagFilter(VK_FILTER_LINEAR)
				->withMinFilter(VK_FILTER_LINEAR)
				->Build();

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
					->withLayerCount(1)
					->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
					->withWidthHeight(extent.width, extent.height)
					->addViewAttachment(sceneLight->shadowMap[1], i, 1)
					->Build()
				);
			}
		}

		//ライトをまとめて一つのVkDescriptorSetを管理する
		sceneLight->texDescriptorSet = descriptorSetFactory->Create
		(
			descriptorSetBuilder->initProperty()
			->withDescriptorSetCount(1)
			->withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::SHADOWMAP))
			->withBindingBuffer(0)
			->withBuffer(sceneLight->uniformBuffer[0])
			->withRange(sizeof(PointLightUniform))
			->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			->addBufferInfo()
			->withBindingBuffer(1)
			->withBuffer(sceneLight->uniformBuffer[1])
			->withRange(sizeof(DirectionLightUniform))
			->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			->addBufferInfo()
			->withBindingImage(2)
			->withTexture(sceneLight->shadowMap[0])
			->withImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			->addImageInfo()
			->withBindingImage(3)
			->withTexture(sceneLight->shadowMap[1])
			->withImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			->addImageInfo()
			->Build()
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

						sceneLight->pointUniform.position[comp.index] = glm::vec4(comp.position, 0.0f);
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

						sceneLight->dirUniform.position[comp.index] = glm::vec4(comp.position, 0.0f);
						sceneLight->dirUniform.direction[comp.index] = glm::vec4(comp.direction, 0.0f);
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
						->withImagePath("textures\\grass.hdr")
						->Build()
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
							->withDescriptorSetCount(1)
							->withDescriptorSetLayout(descriptorSetLayoutFactory->Create(LayoutPattern::CAMERA))
							->withBindingBuffer(0)
							->withBuffer(comp.uniform)
							->withRange(sizeof(CameraUniform))
							->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
							->addBufferInfo()
							->Build()
						);
				}
			}
		);
}

//コンポーネントの更新処理
void GameManager::OnUpdate()
{
	ecsManager->RunFunction<InputComp>
		(
			{
				[&](InputComp& input)
				{
					input.lastFrame.copy(input.currentFrame);
				}
			}
		);

	ecsManager->RunFunction<PhysicComp, TransformComp>
		(
			{
				[&](PhysicComp& physic,TransformComp& transform)
				{
					transform.position += physic.gravity;
				}
			}
		);
}

//更新処理後の処理
void GameManager::OnLateUpdate()
{
	ecsManager->RunFunction<PhysicComp, TransformComp, InputComp>
		(
			{
				[&](PhysicComp& physic,TransformComp& transform,InputComp& input)
				{
					int w = inputSystem->getKey(GLFW_KEY_W);
					int s = inputSystem->getKey(GLFW_KEY_S);
					int a = inputSystem->getKey(GLFW_KEY_A);
					int d = inputSystem->getKey(GLFW_KEY_D);

					if (w == GLFW_PRESS)
					{
						physic.velocity.velocity = glm::vec3(0.0f, 0.0f, 1.0f);
					}

					if (s == GLFW_PRESS)
					{
						physic.velocity.velocity = glm::vec3(0.0f, 0.0f, -1.0f);
					}

					if (a == GLFW_PRESS)
					{
						physic.velocity.velocity = glm::vec3(-1.0f, 0.0f, 0.0f);
					}

					if (d == GLFW_PRESS)
					{
						physic.velocity.velocity = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					transform.position += physic.velocity.velocity;
				}
			}
		);

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

	ecsManager->RunFunction<CameraComp,InputComp>
		(
			{
				[&](CameraComp& camera,InputComp& input)
				{
					
				}
			}
		);

	ecsManager->RunFunction<TargetEntityComp, CameraComp>
		(
			{
				[&](TargetEntityComp& targetComp,CameraComp& cameraComp)
				{
					TransformComp* target = ecsManager->GetComponent<TransformComp>(targetComp.targetEntity);

					cameraComp.matrices.position = target->position + targetComp.offset;

					cameraComp.matrices.view = glm::lookAt(cameraComp.matrices.position, target->position, glm::vec3(0.0f, -1.0f, 0.0f));
					cameraComp.matrices.proj = glm::perspective(cameraComp.viewAngle, cameraComp.aspect, cameraComp.zNear, cameraComp.zFar);

					memcpy(cameraComp.uniform->mappedPtr, &cameraComp.matrices, sizeof(CameraUniform));
				}
			}
		);

	ecsManager->RunFunction<DirectionLightComp>
		(
			{
				[&](DirectionLightComp& dirLightComp)
				{
					sceneLight->dirUniform.lightCount
						= static_cast<uint32_t>(ecsManager->GetEntitiesWithComponents<DirectionLightComp>().size());

					sceneLight->dirUniform.position[dirLightComp.index] = glm::vec4(dirLightComp.position, 0.0f);

					sceneLight->dirUniform.color[dirLightComp.index] = dirLightComp.color;

					sceneLight->dirUniform.direction[dirLightComp.index] = glm::vec4(dirLightComp.direction, 0.0f);

					sceneLight->dirUniform.viewProj[dirLightComp.index] = glm::ortho(-500.0f, 500.0f, 500.0f, -500.0f, 0.1f, 1000.0f)
						* glm::lookAt(dirLightComp.position, dirLightComp.position + dirLightComp.direction, glm::vec3(0.0f, 1.0f, 0.0f));
				}
			}
		);

	ecsManager->RunFunction<TransformComp, MeshRendererComp>
		(
			{
			[&](TransformComp& transComp,MeshRendererComp& rendererComp)
			{
				glm::mat4 rotateMat = glm::mat4(1.0f);
				rotateMat *= glm::rotate(glm::mat4(1.0f), glm::radians(transComp.rotation[0]), glm::vec3(1, 0, 0));
				rotateMat *= glm::rotate(glm::mat4(1.0f), glm::radians(transComp.rotation[1]), glm::vec3(0, 1, 0));
				rotateMat *= glm::rotate(glm::mat4(1.0f), glm::radians(transComp.rotation[2]), glm::vec3(0, 0, 1));

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

					float deltaTime = (clock() - animComp.startTime) / 1000.0f;
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


	memcpy(sceneLight->uniformBuffer[0]->mappedPtr, &sceneLight->pointUniform, sizeof(PointLightUniform));
	memcpy(sceneLight->uniformBuffer[1]->mappedPtr, &sceneLight->dirUniform, sizeof(DirectionLightUniform));
}

//オブジェクトのレンダリング
void GameManager::Rendering()
{
	const uint32_t frameIndex = swapChain->getCurrentFrameIndex();

	{
		//シャドウマップを作成する
		ecsManager->RunFunction<DirectionLightComp>
			(
				{
					[&](DirectionLightComp& comp)
					{
						std::shared_ptr<CommandBuffer> commandBuffer
							= std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandBufferFactory);

						commandBuffer->setCommandBufffer(commandBufferFactory->createCommandBuffer(1))
							->setSemaphore(commandBufferFactory->createSemaphore(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

						commandBuffer->recordBegin();

						const VkExtent2D extent = swapChain->getSwapChainExtent();

						const RenderProperty renderProp = render->initProperty()
							->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
							->withFrameBuffer(sceneLight->frameBuffer[1][comp.index])
							->withCommandBuffer(commandBuffer)
							->withRenderArea(extent.width, extent.height)
							->withClearDepth(1.0f)
							->withClearStencil(0)
							->Build();

						render->RenderStart(renderProp);

						ecsManager->RunFunction<GltfModelComp, MeshRendererComp>
							(
								{
									[&](GltfModelComp& modelComp,MeshRendererComp& rendererComp)
									{
										std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(modelComp.modelID);
										if (!gltfModel)
										{
											throw std::runtime_error("GameManager::std::shared_ptr<Render>::gltfModel is nullptr");
										}

										vkCmdBindPipeline(commandBuffer->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
											pipelineFactory->Create(PipelinePattern::CALC_SHADOWMAP)->pipeline);

										vkCmdSetDepthBias(commandBuffer->getCommand(), 1.25f, 0.0f, 1.75f);

										VkViewport viewport{};
										viewport.x = 0.0f;
										viewport.y = 0.0f;
										viewport.width = (float)swapChain->getSwapChainExtent().width;
										viewport.height = (float)swapChain->getSwapChainExtent().height;
										viewport.minDepth = 0.0f;
										viewport.maxDepth = 1.0f;
										vkCmdSetViewport(commandBuffer->getCommand(), 0, 1, &viewport);

										VkRect2D scissor{};
										scissor.offset = { 0, 0 };
										scissor.extent = swapChain->getSwapChainExtent();
										vkCmdSetScissor(commandBuffer->getCommand(), 0, 1, &scissor);

										std::array<VkDescriptorSet, 2> descriptorSets;

										VkDeviceSize offsets[] = { 0 };

										vkCmdPushConstants(commandBuffer->getCommand(), pipelineLayoutFactory->Create(PipelineLayoutPattern::CALC_SHADOWMAP)->pLayout
											, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &comp.index);

										for (int i = 0; i < gltfModel->nodes.size(); i++)
										{
											const Mesh& mesh = gltfModel->nodes[i].mesh;

											if (mesh.vertices.size() != 0)
											{
												vkCmdBindVertexBuffers(commandBuffer->getCommand(), 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

												vkCmdBindIndexBuffer(commandBuffer->getCommand(), gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

												for (const auto& primitive : mesh.primitives)
												{
													std::shared_ptr<Material> material = gltfModel->materials[primitive.materialIndex];

													descriptorSets[0] = rendererComp.modelAnimDesc[i]->descriptorSet;
													descriptorSets[1] = sceneLight->uniformDescriptorSet[1]->descriptorSet;

													vkCmdBindDescriptorSets(commandBuffer->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
														pipelineLayoutFactory->Create(PipelineLayoutPattern::CALC_SHADOWMAP)->pLayout, 0
														, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

													vkCmdDrawIndexed(commandBuffer->getCommand(), primitive.indexCount, 1, primitive.firstIndex, 0, 0);
												}
											}
										}
									}
								}
							);

						render->RenderEnd(renderProp);

						commandBuffer->recordEnd();

						commandBuffer->Submit(vulkanCore->getGraphicsQueue());

						renderCommand[frameIndex]->addWaitCommand(commandBuffer);
					}
				}
			);
	}

	std::shared_ptr<FrameBuffer> frameBuffer = swapChain->getCurrentFrameBuffer();

	{
		ecsManager->RunFunction<SkyDomeComp>
			(
				{
					[&](SkyDomeComp& comp)
					{
						std::shared_ptr<SkyDome> skydome = skydomeFactory->GetSkyDome(comp.ID);

						std::shared_ptr<CommandBuffer> commandBuffer
							= std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandBufferFactory);

						commandBuffer->setCommandBufffer(commandBufferFactory->createCommandBuffer(1))
							->setSemaphore(commandBufferFactory->createSemaphore(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

						commandBuffer->recordBegin();

						const VkExtent2D extent = swapChain->getSwapChainExtent();

						const RenderProperty renderProp = render->initProperty()
							->withRenderPass(renderPassFactory->Create(RenderPassPattern::CUBEMAP))
							->withFrameBuffer(frameBuffer)
							->withCommandBuffer(commandBuffer)
							->withRenderArea(extent.width, extent.height)
							->withClearColor(glm::vec4(0.0f))
							->withClearDepth(1.0f)
							->withClearStencil(0)
							->Build();

						render->RenderStart(renderProp);

						ecsManager->RunFunction<CameraComp>
							(
								{
									[&](CameraComp& cameraComp)
									{
										const uint32_t modelID = skydomeFactory->getCubemapModelID();

										std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(modelID);
										if (!gltfModel)
										{
											throw std::runtime_error("GameManager::std::shared_ptr<Render>::gltfModel is nullptr");
										}

										vkCmdBindPipeline(commandBuffer->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
											pipelineFactory->Create(PipelinePattern::CUBEMAP)->pipeline);

										VkViewport viewport{};
										viewport.x = 0.0f;
										viewport.y = 0.0f;
										viewport.width = (float)swapChain->getSwapChainExtent().width;
										viewport.height = (float)swapChain->getSwapChainExtent().height;
										viewport.minDepth = 0.0f;
										viewport.maxDepth = 1.0f;
										vkCmdSetViewport(commandBuffer->getCommand(), 0, 1, &viewport);

										VkRect2D scissor{};
										scissor.offset = { 0, 0 };
										scissor.extent = swapChain->getSwapChainExtent();
										vkCmdSetScissor(commandBuffer->getCommand(), 0, 1, &scissor);

										std::array<VkDescriptorSet, 2> descriptorSets;

										VkDeviceSize offsets[] = { 0 };

										for (int i = 0; i < gltfModel->nodes.size(); i++)
										{
											const Mesh& mesh = gltfModel->nodes[i].mesh;

											if (mesh.vertices.size() != 0)
											{
												vkCmdBindVertexBuffers(commandBuffer->getCommand(), 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

												vkCmdBindIndexBuffer(commandBuffer->getCommand(), gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

												for (const auto& primitive : mesh.primitives)
												{
													std::shared_ptr<Material> material = gltfModel->materials[primitive.materialIndex];

													descriptorSets[0] = cameraComp.descriptorSet->descriptorSet;
													descriptorSets[1] = skydome->cubemapDescriptorSet->descriptorSet;

													vkCmdBindDescriptorSets(commandBuffer->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
														pipelineLayoutFactory->Create(PipelineLayoutPattern::CUBEMAP)->pLayout, 0
														, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

													vkCmdDrawIndexed(commandBuffer->getCommand(), primitive.indexCount, 1, primitive.firstIndex, 0, 0);
												}
											}
										}
									}
								}
							);

						render->RenderEnd(renderProp);

						commandBuffer->recordEnd();

						commandBuffer->Submit(vulkanCore->getGraphicsQueue());

						renderCommand[frameIndex]->addWaitCommand(commandBuffer);
					}
				}
			);
	}

	{
		std::function<void(GltfModelComp&, MeshRendererComp&)> renderModel =
			[&](GltfModelComp& gltfComp, MeshRendererComp& meshRendererComp)
			{
				std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(gltfComp.modelID);
				if (!gltfModel)
				{
					throw std::runtime_error("GameManager::Render::gltfModel is nullptr");
				}

				vkCmdBindPipeline(renderCommand[frameIndex]->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineFactory->Create(PipelinePattern::PBR)->pipeline);

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)swapChain->getSwapChainExtent().width;
				viewport.height = (float)swapChain->getSwapChainExtent().height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(renderCommand[frameIndex]->getCommand(), 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = swapChain->getSwapChainExtent();
				vkCmdSetScissor(renderCommand[frameIndex]->getCommand(), 0, 1, &scissor);

				FragmentParam param{};
				param.alphaness = 1.0f;
				vkCmdPushConstants(renderCommand[frameIndex]->getCommand(), pipelineLayoutFactory->Create(PipelineLayoutPattern::PBR)->pLayout
					, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(FragmentParam), &param);

				std::array<VkDescriptorSet, 5> descriptorSets;

				VkDeviceSize offsets[] = { 0 };

				for (int i = 0; i < gltfModel->nodes.size(); i++)
				{
					const Mesh& mesh = gltfModel->nodes[i].mesh;

					if (mesh.vertices.size() != 0)
					{
						vkCmdBindVertexBuffers(renderCommand[frameIndex]->getCommand(), 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

						vkCmdBindIndexBuffer(renderCommand[frameIndex]->getCommand(), gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

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
							descriptorSets[4] = skydome->iblDescriptorSet->descriptorSet;

							vkCmdBindDescriptorSets(renderCommand[frameIndex]->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayoutFactory->Create(PipelineLayoutPattern::PBR)->pLayout, 0
								, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

							vkCmdDrawIndexed(renderCommand[frameIndex]->getCommand(), primitive.indexCount, 1, primitive.firstIndex, 0, 0);
						}
					}
				}
			};

		renderCommand[frameIndex]->recordBegin();

		const RenderProperty property = render->initProperty()
			->withRenderPass(renderPassFactory->Create(RenderPassPattern::PBR))
			->withFrameBuffer(frameBuffer)
			->withCommandBuffer(renderCommand[frameIndex])
			->withRenderArea(swapChain->getSwapChainExtent().width, swapChain->getSwapChainExtent().height)
			->withClearColor({ 0.0, 0.0, 0.0, 1.0 })
			->withClearDepth(1.0f)
			->withClearStencil(0)
			->Build();

		render->RenderStart(property);

		ecsManager->RunFunction<GltfModelComp, MeshRendererComp>(renderModel);

		render->RenderEnd(property);

		renderCommand[frameIndex]->recordEnd();

		{
			//一つ前のフレームのレンダリングの終了を待ち
			//コマンドバッファをリセットしておく

			const uint32_t lastFrame = (frameIndex == 0) ? 1 : 0;

			renderCommand[lastFrame]->waitFence();
		}

		swapChain->flipSwapChainImage(renderCommand[frameIndex]);
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
	for (auto& command : renderCommand)
	{
		command->releaseMyPtr();
	}

    //ゲームを終了させる
    FinishGame();
}

//ゲーム全体の終了処理
void GameManager::FinishGame()
{
	vulkanCore->waitForGpuIdle();
}