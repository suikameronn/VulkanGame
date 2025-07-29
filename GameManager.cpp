#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//�Q�[���S�̂̏���������
{
	vulkanCore = std::make_shared<VulkanCore>(window);

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//�w�肵��fps����A��t���[���ɂ������鎞�Ԃ��v�Z����

    bool load = false;

	ecsManager = std::make_shared<ECSManager>();

	createInstance();//�C���X�^���X�̍쐬

	createScene();//�V�[���̍쐬

    mainGameLoop();//�Q�[���̃��C�����[�v���J�n
}

//�C���X�^���X���쐬����
void GameManager::createInstance()
{
    createBuilder();//�r���_�[�̗p��
    createFactory();//�t�@�N�g���[�̗p��

	VkDevice& device = vulkanCore->getLogicDevice();

	pipelineBuilder = std::make_shared<PipelineBuilder>(device, shaderFactory);
	pipelineFactory = std::make_shared<PipelineFactory>(device, pipelineLayoutFactory, shaderFactory
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
	pipelineFactory = std::make_shared<PipelineFactory>(device, pipelineLayoutFactory, shaderFactory, pipelineBuilder, renderPassFactory);
}

//�V�[���̍쐬
void GameManager::createScene()
{
	size_t entity1 = ecsManager->GenerateEntity();

	ecsManager->AddComponent<TransformComp>(entity1);
	GltfModelComp* comp = ecsManager->AddComponent<GltfModelComp>(entity1);
	ecsManager->AddComponent<GltfModelAnimComp>(entity1);
	ecsManager->AddComponent<MeshRendererComp>(entity1);

	comp->filePath = "models/PlayerModel.glb";
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
            std::cout << "fps down" << std::endl;
        }
        start = std::chrono::system_clock::now();
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
}

//�R���|�[�l���g�̍X�V����
void GameManager::OnUpdate()
{

}

//�X�V������̏���
void GameManager::OnLateUpdate()
{

}

//�I�u�W�F�N�g�̃����_�����O
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

				std::array<VkDescriptorSet, 1> descriptorSets;

				VkDeviceSize offsets[] = { 0 };

				for (const auto& node : gltfModel->nodes)
				{
					for (const auto& mesh : node.meshArray)
					{
						vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &gltfModel->getVertBuffer(node.offset)->buffer, offsets);

						vkCmdBindIndexBuffer(commandBuffer->commandBuffer, gltfModel->getIndeBuffer(node.offset)->buffer, 0, VK_INDEX_TYPE_UINT32);

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
    //�Q�[�����I��������
    FinishGame();
}

//�Q�[���S�̂̏I������
void GameManager::FinishGame()
{

}