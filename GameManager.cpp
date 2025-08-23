#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//�Q�[���S�̂̏���������
{
	vulkanCore = std::make_shared<VulkanCore>(window);

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//�w�肵��fps����A��t���[���ɂ������鎞�Ԃ��v�Z����

    bool load = false;

	ecsManager = std::make_shared<ECSManager>();

	createInstance();//�C���X�^���X�̍쐬

	sceneLight = std::make_shared<SceneLight>();

	createScene();//�V�[���̍쐬

	createRenderCommand();//�����_�[�p�R�}���h�o�b�t�@�̍쐬

    mainGameLoop();//�Q�[���̃��C�����[�v���J�n
}

//�C���X�^���X���쐬����
void GameManager::createInstance()
{
    createBuilder();//�r���_�[�̗p��
    createFactory();//�t�@�N�g���[�̗p��

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
}

//�r���_�[�̗p��
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

//�t�@�N�g���[�̗p��
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

//�V�[���̍쐬
void GameManager::createScene()
{
	size_t entity1 = ecsManager->GenerateEntity();

	ecsManager->AddComponent<TransformComp>(entity1)->scale = glm::vec3(200.0f);

	GltfModelComp* comp = ecsManager->AddComponent<GltfModelComp>(entity1);

	GltfModelAnimComp* animComp = ecsManager->AddComponent<GltfModelAnimComp>(entity1);
	animComp->animationName = "Idle";

	ecsManager->AddComponent<MeshRendererComp>(entity1);

	comp->filePath = "models/robot.glb";

	size_t entity2 = ecsManager->GenerateEntity();
	DirectionLightComp* dLight = ecsManager->AddComponent<DirectionLightComp>(entity2);
	dLight->position = glm::vec3(200.0f, 200.0f, 200.0f);
	dLight->color = glm::vec4(1.0f);
	dLight->direction = glm::vec3(-200.0f, -200.0f, -200.0f);
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

	/*size_t entity5 = ecsManager->GenerateEntity();
	DirectionLightComp* dLight2 = ecsManager->AddComponent<DirectionLightComp>(entity5);
	dLight2->position = glm::vec3(200.0f, 200.0f, 200.0f);
	dLight2->color = glm::vec4(1.0f);
	dLight2->direction = glm::vec3(-200.0f, -200.0f, -200.0f);
	*/
}

//�����_�[�p�R�}���h�o�b�t�@�̍쐬
void GameManager::createRenderCommand()
{
	renderCommand.resize(2);

	//���炩����VkCommandBuffer��VkFence���쐬���Ă���
	for (auto& command : renderCommand)
	{
		command = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandBufferFactory);

		command->setCommandBufffer(commandBufferFactory->createCommandBuffer(1))
			->setFence(commandBufferFactory->createFence())
			->setSemaphore(commandBufferFactory->createSemaphore(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	}
}

void GameManager::mainGameLoop()//���C���Q�[�����[�v
{

    start = std::chrono::system_clock::now();//�t���[�����Ԃ��v��

	OnStart();

    while (true)
    {

        if (vulkanCore->isShouldCloseWindow())//�V�[�����I�����邩�ǂ���
        {
            break;
        }

		OnUpdate();//�R���|�[�l���g�̍X�V����

		OnLateUpdate();//�X�V������̏���

		Rendering();//�I�u�W�F�N�g�̃����_�����O

		OnFrameEnd();//�t���[���I�����̏���

        end = std::chrono::system_clock::now();//�t���[���I�����Ԃ��v��
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();//�t���[���Ԏ��Ԃ��v�Z����
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//fps�𐧌�����
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

//�R���|�[�l���g�̏��񏈗�
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
						//���f���s��p�̃o�b�t�@�����

						rendererComp.modelMatBuffer =
							bufferFactory->Create(sizeof(ModelMat), BufferUsage::UNIFORM, BufferTransferType::NONE);
					}

					{
						//�m�[�h���Ƃ̍s��ȂǗp�̃o�b�t�@�����
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
						//�m�[�h�S�̂̃A�j���[�V�����s��p�̃o�b�t�@�����

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

	{
		{
			int pointLightCount = static_cast<int>(ecsManager->GetEntitiesWithComponents<PointLightComp>().size());

			//���C�g�̐������V���h�E�}�b�v�̃e�N�X�`���̃��C���[�����쐬����

			const VkExtent2D extent = swapChain->getSwapChainExtent();

			//���C�g�R���|�[�l���g�������Ă��A���Ȃ��Ƃ���͂�����̂Ƃ���
			//���\�[�X���쐬����
			pointLightCount = (pointLightCount > 0) ? pointLightCount : 1;

			//�|�C���g���C�g�p�̃��j�t�H�[���o�b�t�@�����
			//����̃��C�g�́A���ׂĈ�̃o�b�t�@�ɔz��Ƃ��Ċi�[�����
			sceneLight->uniformBuffer[0] = bufferFactory->Create
			(
				sizeof(PointLightUniform),
				&sceneLight->pointUniform,
				BufferUsage::UNIFORM,
				BufferTransferType::NONE
			);

			//���j�t�H�[���o�b�t�@��VkDescriptorSet���쐬����
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

			//�|�C���g���C�g�̗p�̕����̃��C���[�����V���h�E�}�b�v�p�̃e�N�X�`�����쐬����
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

			//�e�r���[����t���[���o�b�t�@���쐬����
			//�e�N�X�`���̊e���C���[�ɏ������߂�悤�ɂȂ�
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

			//���C�g�R���|�[�l���g�������Ă��A���Ȃ��Ƃ���͂�����̂Ƃ���
			//���\�[�X���쐬����
			directionLightCount = (directionLightCount > 0) ? directionLightCount : 1;

			const VkExtent2D extent = swapChain->getSwapChainExtent();

			//�f�B���N�V�������C�g�p�̃��j�t�H�[���o�b�t�@�����
			//����̃��C�g�́A���ׂĈ�̃o�b�t�@�ɔz��Ƃ��Ċi�[�����
			sceneLight->uniformBuffer[1] = bufferFactory->Create
			(
				sizeof(DirectionLightUniform),
				&sceneLight->dirUniform,
				BufferUsage::UNIFORM,
				BufferTransferType::NONE
			);

			//���j�t�H�[���o�b�t�@��VkDescriptorSet���쐬����
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

			//�f�B���N�V�������C�g�̗p�̕����̃��C���[�����V���h�E�}�b�v�p�̃e�N�X�`�����쐬����
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

			//�e�r���[����t���[���o�b�t�@���쐬����
			//�e�N�X�`���̊e���C���[�ɏ������߂�悤�ɂȂ�
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

		//���C�g���܂Ƃ߂Ĉ��VkDescriptorSet���Ǘ�����
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
		//���C�g�R���|�[�l���g����e���C�g�̃p�����[�^�����o��
		//��ō쐬�����o�b�t�@�ɓ���Ă���

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

//�R���|�[�l���g�̍X�V����
void GameManager::OnUpdate()
{

}

//�X�V������̏���
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


	memcpy(sceneLight->uniformBuffer[0]->mappedPtr, &sceneLight->pointUniform, sizeof(PointLightUniform));
	memcpy(sceneLight->uniformBuffer[1]->mappedPtr, &sceneLight->dirUniform, sizeof(DirectionLightUniform));
}

//�I�u�W�F�N�g�̃����_�����O
void GameManager::Rendering()
{
	const uint32_t frameIndex = swapChain->getCurrentFrameIndex();

	{
		//�V���h�E�}�b�v���쐬����
		ecsManager->RunFunction<DirectionLightComp>
			(
				{
					[&](DirectionLightComp& comp)
					{
						std::shared_ptr<CommandBuffer> commandBuffer
							= std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandBufferFactory);

						commandBuffer->setCommandBufffer(commandBufferFactory->createCommandBuffer(1))
							//->setSemaphore(commandBufferFactory->createSemaphore(),VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
						->setFence(commandBufferFactory->createFence());

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

										//vkCmdSetDepthBias(commandBuffer->getCommand(), 1.25f, 0.0f, 1.75f);

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

						commandBuffer->waitFence();

						//renderCommand[frameIndex]->addWaitCommand(commandBuffer);
					}
				}
			);
	}

	{
		std::shared_ptr<FrameBuffer> frameBuffer = swapChain->getCurrentFrameBuffer();

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
							descriptorSets[4] = skydome->descriptorSet->descriptorSet;

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
			//��O�̃t���[���̃����_�����O�̏I����҂�
			//�R�}���h�o�b�t�@�����Z�b�g���Ă���

			const uint32_t lastFrame = (frameIndex == 0) ? 1 : 0;

			renderCommand[lastFrame]->waitFence();
		}

		swapChain->flipSwapChainImage(renderCommand[frameIndex]);
	}


	//���\�[�X�̔j��
	descriptorSetLayoutFactory->resourceDestruct();
	bufferFactory->resourceDestruct();
	frameBufferFactory->resourceDestruct();
	pipelineLayoutFactory->resourceDestruct();
	pipelineFactory->resourceDestruct();
	renderPassFactory->resourceDestruct();
	shaderFactory->resourceDestruct();
	textureFactory->resourceDestruct();
}

//�t���[���I�����̏���
void GameManager::OnFrameEnd()
{

}

//�V�[�����I�������鏈��
void GameManager::exitScene()
{
	for (auto& command : renderCommand)
	{
		command->releaseMyPtr();
	}

    //�Q�[�����I��������
    FinishGame();
}

//�Q�[���S�̂̏I������
void GameManager::FinishGame()
{
	vulkanCore->waitForGpuIdle();
}