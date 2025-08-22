#include"SkyDomeBuilder.h"

SkyDomeBuilder::SkyDomeBuilder(std::shared_ptr<VulkanCore> core, std::shared_ptr<GpuBufferFactory> buffer, std::shared_ptr<TextureFactory> texture
	, std::shared_ptr<FrameBufferFactory> frame, std::shared_ptr<PipelineLayoutFactory> pLayout
	, std::shared_ptr<PipelineFactory> pipeline, std::shared_ptr<RenderPassFactory> renderPass
	, std::shared_ptr<DescriptorSetLayoutFactory> layout, std::shared_ptr<DescriptorSetFactory> descriptorSet
	, std::shared_ptr<Render> render, std::shared_ptr<GltfModelFactory> model
	, std::shared_ptr<CommandBufferFactory> command)
{
	vulkanCore = core;

	bufferFactory = buffer;

	textureFactory = texture;

	frameBufferFactory = frame;

	layoutFactory = layout;

	descriptorSetFactory = descriptorSet;

	pipelineLayoutFactory = pLayout;

	pipelineFactory = pipeline;

	renderPassFactory = renderPass;

	this->render = render;

	modelFactory = model;

	commandFactory = command;

	cubemapModelID = modelFactory->Load("models/cubemap.glb");
}

//マップの元となる画像ファイルを設定する
std::shared_ptr<SkyDomeBuilder> SkyDomeBuilder::withImagePath(const std::string path)
{
	property.srcImagePath = path;

	return shared_from_this();
}

//キューブマップ作成時のビューの向きを切り替える
void SkyDomeBuilder::switchView(std::shared_ptr<DescriptorSet> uniform, const int index)
{
	CameraUniform camera{};
	camera.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	switch (CUBEMAP_VIEW(index))
	{
	case CUBEMAP_VIEW::FRONT:
		camera.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		break;
	case CUBEMAP_VIEW::BACK:
		camera.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		break;
	case CUBEMAP_VIEW::RIGHT:
		camera.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		break;
	case CUBEMAP_VIEW::LEFT:
		camera.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		break;
	case CUBEMAP_VIEW::TOP:
		camera.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		break;
	case CUBEMAP_VIEW::BOTTOM:
		camera.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		break;
	}

	memcpy(uniform->buffer[0]->mappedPtr, &camera, sizeof(CameraUniform));
}

//各種マップを作成する
void SkyDomeBuilder::createBackGround(const std::shared_ptr<SkyDome> skydome,const std::shared_ptr<Texture> srcTexture
	, const std::array<std::shared_ptr<DescriptorSet>,CUBEMAP_LAYER>& uniform, const std::shared_ptr<DescriptorSet> srcTex)
{
	BackGroundColor& backGround = skydome->backGround;
	
	const TextureProperty texProp = srcTexture->property;
	const VkExtent3D extent = srcTexture->property.image.info.extent;
	const uint32_t size = std::min(extent.width, extent.height);

	//SamplerCubeの6つのレイヤーを持つテクスチャを作成
	textureFactory->getBuilder()->initProperty()
		->withWidthHeight(size, size, 1)
		->withImageType(VK_IMAGE_TYPE_2D)
		->withUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		->withTiling(VK_IMAGE_TILING_OPTIMAL)
		->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
		->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		->withLayerCount(1)
		->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
		->withViewType(VK_IMAGE_VIEW_TYPE_2D)
		->withTargetLayer(0, 1)
		->addView()
		->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
		->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
		->withMagFilter(VK_FILTER_LINEAR)
		->withMinFilter(VK_FILTER_LINEAR);

	std::array<std::shared_ptr<Texture>, CUBEMAP_LAYER> multiLayerTex;
	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		multiLayerTex[i] = textureFactory->Create
		(
			textureFactory->getBuilder()->Build()
		);
	}

	//上で作ったテクスチャの各レイヤーをレンダーターゲットとして
	//フレームバッファを作る
	std::array<std::shared_ptr<FrameBuffer>, CUBEMAP_LAYER> frameBufferArray;
	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		frameBufferArray[i] = frameBufferFactory->Create
		(
			frameBufferFactory->getBuilder()->initProperty()
			->withLayerCount(1)
			->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_CUBEMAP))
			->withWidthHeight(size, size)
			->addViewAttachment(multiLayerTex[i])
			->Build()
		);
	}

	//各フレームバッファにレンダリングしていく
	std::shared_ptr<CommandBuffer> renderCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);

	renderCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setSemaphore(commandFactory->createSemaphore(), VK_PIPELINE_STAGE_TRANSFER_BIT);

	renderCommand->recordBegin();

	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		const RenderProperty renderProp = render->initProperty()
			->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_CUBEMAP))
			->withFrameBuffer(frameBufferArray[i])
			->withCommandBuffer(renderCommand)
			->withRenderArea(size, size)
			->withClearDepth(1.0f)
			->withClearStencil(0)
			->Build();

		render->RenderStart(renderProp);

		std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(cubemapModelID);
		if (!gltfModel)
		{
			throw std::runtime_error("GameManager::std::shared_ptr<Render>::gltfModel is nullptr");
		}

		vkCmdBindPipeline(renderCommand->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineFactory->Create(PipelinePattern::CALC_CUBEMAP)->pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)size;
		viewport.height = (float)size;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(renderCommand->getCommand(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { size,size };
		vkCmdSetScissor(renderCommand->getCommand(), 0, 1, &scissor);

		std::array<VkDescriptorSet, 2> descriptorSets;

		VkDeviceSize offsets[] = { 0 };

		for (int j = 0; j < gltfModel->nodes.size(); j++)
		{
			const Mesh& mesh = gltfModel->nodes[j].mesh;

			if (mesh.vertices.size() != 0)
			{
				vkCmdBindVertexBuffers(renderCommand->getCommand(), 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

				vkCmdBindIndexBuffer(renderCommand->getCommand(), gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

				for (const auto& primitive : mesh.primitives)
				{
					descriptorSets[0] = uniform[i]->descriptorSet;
					descriptorSets[1] = srcTex->descriptorSet;

					vkCmdBindDescriptorSets(renderCommand->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
						pipelineLayoutFactory->Create(PipelineLayoutPattern::CALC_CUBEMAP)->pLayout, 0
						, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

					vkCmdDrawIndexed(renderCommand->getCommand(), primitive.indexCount, 1, primitive.firstIndex, 0, 0);
				}
			}
		}

		render->RenderEnd(renderProp);
	}

	renderCommand->recordEnd();

	renderCommand->Submit(vulkanCore->getGraphicsQueue());

	backGround.multiLayerTex = textureFactory->Create
	(
		textureFactory->getBuilder()->initProperty()
		->withWidthHeight(size, size, 1)
		->withImageType(VK_IMAGE_TYPE_2D)
		->withImageFlag(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		->withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		->withTiling(VK_IMAGE_TILING_OPTIMAL)
		->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
		->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		->withLayerCount(CUBEMAP_LAYER)
		->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		->withFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
		->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
		->withTargetLayer(0, CUBEMAP_LAYER)
		->addView()
		->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
		->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
		->withMagFilter(VK_FILTER_LINEAR)
		->withMinFilter(VK_FILTER_LINEAR)
		->Build()
	);

	std::shared_ptr<CommandBuffer> copyCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);

	copyCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setSemaphore(commandFactory->createSemaphore(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
		->addWaitCommand(renderCommand);

	copyCommand->recordBegin();

	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		textureFactory->getCopy()->Copy
		(
			textureFactory->getCopy()->initProperty()
			->withAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
			->withSrcLayerRange(0, 1)
			->withDstLayerRange(i, 1)
			->withSrcMipmapLevel(0)
			->withDstMipmapLevel(0)
			->withSize(size, size)
			->withCommandBuffer(copyCommand)
			->withSrcTexture(multiLayerTex[i])
			->withDstTexture(backGround.multiLayerTex)
			->Build()
		);
	}

	copyCommand->recordEnd();

	copyCommand->Submit(vulkanCore->getGraphicsQueue());

	std::shared_ptr<CommandBuffer> transCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);
	transCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->addWaitCommand(copyCommand)
		->setFence(commandFactory->createFence());

	transCommand->recordBegin();

	textureFactory->getBuilder()->transitionImageLayout
	(
		backGround.multiLayerTex->image,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		1,
		CUBEMAP_LAYER,
		transCommand
	);

	transCommand->recordEnd();

	transCommand->Submit(vulkanCore->getGraphicsQueue());

	transCommand->waitFence();
}

void SkyDomeBuilder::createDiffuse(const std::shared_ptr<SkyDome> skydome, const std::shared_ptr<Texture> srcTexture
	, const std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER>& uniform, const std::shared_ptr<DescriptorSet> srcTex)
{
	DiffuseMap& diffuse = skydome->diffuse;

	const TextureProperty texProp = srcTexture->property;
	const VkExtent3D extent = srcTexture->property.image.info.extent;
	const uint32_t size = std::min(extent.width, extent.height);

	//SamplerCubeの6つのレイヤーを持つテクスチャを作成
	textureFactory->getBuilder()->initProperty()
		->withWidthHeight(size, size, 1)
		->withImageType(VK_IMAGE_TYPE_2D)
		->withUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		->withTiling(VK_IMAGE_TILING_OPTIMAL)
		->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
		->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		->withLayerCount(1)
		->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
		->withViewType(VK_IMAGE_VIEW_TYPE_2D)
		->withTargetLayer(0, 1)
		->addView()
		->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
		->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
		->withMagFilter(VK_FILTER_LINEAR)
		->withMinFilter(VK_FILTER_LINEAR);

	std::array<std::shared_ptr<Texture>, CUBEMAP_LAYER> multiLayerTexture;
	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		multiLayerTexture[i] = textureFactory->Create
		(
			textureFactory->getBuilder()->Build()
		);
	}

	std::cout << "Create Image" << std::endl;

	//上で作ったテクスチャの各レイヤーをレンダーターゲットとして
	//フレームバッファを作る
	std::array<std::shared_ptr<FrameBuffer>, CUBEMAP_LAYER> frameBufferArray;
	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		frameBufferArray[i] = frameBufferFactory->Create
		(
			frameBufferFactory->getBuilder()->initProperty()
			->withLayerCount(1)
			->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_DIFFUSE_SPECULAR))
			->withWidthHeight(size, size)
			->addViewAttachment(multiLayerTexture[i])
			->Build()
		);
	}

	std::cout << "Create FrameBuffer" << std::endl;

	//各フレームバッファにレンダリングしていく
	std::shared_ptr<CommandBuffer> renderCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);

	renderCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setFence(commandFactory->createFence());

	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		renderCommand->recordBegin();

		const RenderProperty renderProp = render->initProperty()
			->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_DIFFUSE_SPECULAR))
			->withFrameBuffer(frameBufferArray[i])
			->withCommandBuffer(renderCommand)
			->withRenderArea(size, size)
			->withClearDepth(1.0f)
			->withClearStencil(0)
			->Build();

		render->RenderStart(renderProp);

		std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(cubemapModelID);
		if (!gltfModel)
		{
			throw std::runtime_error("GameManager::std::shared_ptr<Render>::gltfModel is nullptr");
		}

		vkCmdBindPipeline(renderCommand->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineFactory->Create(PipelinePattern::CALC_IBL_DIFFUSE)->pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)size;
		viewport.height = (float)size;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(renderCommand->getCommand(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { size,size };
		vkCmdSetScissor(renderCommand->getCommand(), 0, 1, &scissor);

		std::array<VkDescriptorSet, 2> descriptorSets;

		VkDeviceSize offsets[] = { 0 };

		for (int j = 0; j < gltfModel->nodes.size(); j++)
		{
			const Mesh& mesh = gltfModel->nodes[j].mesh;

			if (mesh.vertices.size() != 0)
			{
				vkCmdBindVertexBuffers(renderCommand->getCommand(), 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

				vkCmdBindIndexBuffer(renderCommand->getCommand(), gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

				for (const auto& primitive : mesh.primitives)
				{
					descriptorSets[0] = uniform[i]->descriptorSet;
					descriptorSets[1] = srcTex->descriptorSet;

					vkCmdBindDescriptorSets(renderCommand->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
						pipelineLayoutFactory->Create(PipelineLayoutPattern::CALC_IBL_DIFFUSE)->pLayout, 0
						, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

					vkCmdDrawIndexed(renderCommand->getCommand(), primitive.indexCount, 1, primitive.firstIndex, 0, 0);
				}
			}
		}

		render->RenderEnd(renderProp);

		renderCommand->recordEnd();

		renderCommand->Submit(vulkanCore->getGraphicsQueue());

		renderCommand->waitFence();
	}

	std::cout << "Render" << std::endl;

	diffuse.multiLayerTex = textureFactory->Create
	(
		textureFactory->getBuilder()->initProperty()
		->withWidthHeight(size, size, 1)
		->withImageType(VK_IMAGE_TYPE_2D)
		->withImageFlag(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		->withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		->withTiling(VK_IMAGE_TILING_OPTIMAL)
		->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
		->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		->withLayerCount(CUBEMAP_LAYER)
		->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		->withFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
		->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
		->withTargetLayer(0, CUBEMAP_LAYER)
		->addView()
		->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
		->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
		->withMagFilter(VK_FILTER_LINEAR)
		->withMinFilter(VK_FILTER_LINEAR)
		->Build()
	);

	std::cout << "Copy " << std::endl;

	std::shared_ptr<CommandBuffer> copyCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);
	copyCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setSemaphore(commandFactory->createSemaphore(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

	copyCommand->recordBegin();

	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		textureFactory->getCopy()->Copy
		(
			textureFactory->getCopy()->initProperty()
			->withAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
			->withSrcLayerRange(0, 1)
			->withDstLayerRange(i, 1)
			->withSrcMipmapLevel(0)
			->withDstMipmapLevel(0)
			->withSize(size, size)
			->withCommandBuffer(copyCommand)
			->withSrcTexture(multiLayerTexture[i])
			->withDstTexture(diffuse.multiLayerTex)
			->Build()
		);
	}

	copyCommand->recordEnd();

	copyCommand->Submit(vulkanCore->getGraphicsQueue());

	std::cout << "Transition" << std::endl;

	std::shared_ptr<CommandBuffer> transCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);
	transCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->addWaitCommand(copyCommand)
		->setFence(commandFactory->createFence());

	transCommand->recordBegin();

	textureFactory->getBuilder()->transitionImageLayout
	(
		diffuse.multiLayerTex->image,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		1,
		CUBEMAP_LAYER,
		transCommand
	);

	transCommand->recordEnd();

	transCommand->Submit(vulkanCore->getGraphicsQueue());

	transCommand->waitFence();
}

void SkyDomeBuilder::createReflection(const std::shared_ptr<SkyDome> skydome, const std::shared_ptr<Texture> srcTexture
	, const std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER>& uniform, const std::shared_ptr<DescriptorSet> srcTex)
{
	ReflectionMap& reflection = skydome->reflection;

	const TextureProperty texProp = srcTexture->property;
	const VkExtent3D extent = srcTexture->property.image.info.extent;
	const uint32_t size = std::min(extent.width, extent.height);

	uint32_t mipmap = size;
	std::vector<uint32_t> mipmapLevelSize;
	while (true)
	{
		mipmapLevelSize.push_back(mipmap);
		mipmap = mipmap / 2;

		if (mipmap < 1)
		{
			break;
		}
	}

	//SamplerCubeの6つのレイヤーを持つテクスチャを作成
	std::vector<std::shared_ptr<Texture>> multiLayerTex(CUBEMAP_LAYER * mipmapLevelSize.size());
	for (int i = 0; i < mipmapLevelSize.size(); i++)
	{
		for (int j = 0; j < CUBEMAP_LAYER; j++)
		{
			multiLayerTex[i * CUBEMAP_LAYER + j] = textureFactory->Create
			(
				textureFactory->getBuilder()->initProperty()
				->withWidthHeight(mipmapLevelSize[i], mipmapLevelSize[i], 1)
				->withImageType(VK_IMAGE_TYPE_2D)
				->withUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
				->withTiling(VK_IMAGE_TILING_OPTIMAL)
				->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
				->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				->withLayerCount(1)
				->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
				->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
				->withViewType(VK_IMAGE_VIEW_TYPE_2D)
				->withTargetLayer(0, 1)
				->withTargetMipmapLevel(0, 1)
				->addView()
				->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
				->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
				->withMagFilter(VK_FILTER_LINEAR)
				->withMinFilter(VK_FILTER_LINEAR)
				->Build()
			);
		}
	}

	//上で作ったテクスチャの各レイヤーをレンダーターゲットとして
	//フレームバッファを作る
	std::vector<std::shared_ptr<FrameBuffer>> frameBufferArray(CUBEMAP_LAYER * mipmapLevelSize.size());

	for(int i = 0;i < mipmapLevelSize.size();i++)
	{
		for (int j = 0; j < CUBEMAP_LAYER; j++)
		{
			frameBufferArray[i * CUBEMAP_LAYER + j] = frameBufferFactory->Create
			(
				frameBufferFactory->getBuilder()->initProperty()
				->withLayerCount(1)
				->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_DIFFUSE_SPECULAR))
				->withWidthHeight(mipmapLevelSize[i], mipmapLevelSize[i])
				->addViewAttachment(multiLayerTex[i * CUBEMAP_LAYER + j])
				->Build()
			);
		}
	}

	//各フレームバッファにレンダリングしていく
	std::shared_ptr<CommandBuffer> renderCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);

	renderCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setFence(commandFactory->createFence());

	for (int i = 0; i < mipmapLevelSize.size(); i++)
	{
		for (int j = 0; j < CUBEMAP_LAYER; j++)
		{
			renderCommand->recordBegin();

			const RenderProperty renderProp = render->initProperty()
				->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_DIFFUSE_SPECULAR))
				->withFrameBuffer(frameBufferArray[i * CUBEMAP_LAYER + j])
				->withCommandBuffer(renderCommand)
				->withRenderArea(mipmapLevelSize[i], mipmapLevelSize[i])
				->withClearDepth(1.0f)
				->withClearStencil(0)
				->Build();

			render->RenderStart(renderProp);

			std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(cubemapModelID);
			if (!gltfModel)
			{
				throw std::runtime_error("GameManager::std::shared_ptr<Render>::gltfModel is nullptr");
			}

			vkCmdBindPipeline(renderCommand->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineFactory->Create(PipelinePattern::CALC_IBL_SPECULAR)->pipeline);

			//ミップマップレベルに応じて、roughnessとしてシェーダに値を渡して、BRDFの値を調整する
			SpecularPushConstant constant{};
			constant.roughness = static_cast<float>(i) / static_cast<float>(mipmapLevelSize.size());
			vkCmdPushConstants(renderCommand->getCommand()
				, pipelineLayoutFactory->Create(PipelineLayoutPattern::CALC_IBL_SPECULAR)->pLayout
				, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SpecularPushConstant), &constant);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mipmapLevelSize[i];
			viewport.height = (float)mipmapLevelSize[i];
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(renderCommand->getCommand(), 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = { mipmapLevelSize[i],mipmapLevelSize[i] };
			vkCmdSetScissor(renderCommand->getCommand(), 0, 1, &scissor);

			std::array<VkDescriptorSet, 2> descriptorSets;

			VkDeviceSize offsets[] = { 0 };

			for (int k = 0; k < gltfModel->nodes.size(); k++)
			{
				const Mesh& mesh = gltfModel->nodes[k].mesh;

				if (mesh.vertices.size() != 0)
				{
					vkCmdBindVertexBuffers(renderCommand->getCommand(), 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

					vkCmdBindIndexBuffer(renderCommand->getCommand(), gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

					for (const auto& primitive : mesh.primitives)
					{
						descriptorSets[0] = uniform[j]->descriptorSet;
						descriptorSets[1] = srcTex->descriptorSet;

						vkCmdBindDescriptorSets(renderCommand->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelineLayoutFactory->Create(PipelineLayoutPattern::CALC_IBL_SPECULAR)->pLayout, 0
							, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

						vkCmdDrawIndexed(renderCommand->getCommand(), primitive.indexCount, 1, primitive.firstIndex, 0, 0);
					}
				}
			}

			render->RenderEnd(renderProp);

			renderCommand->recordEnd();

			renderCommand->Submit(vulkanCore->getGraphicsQueue());

			renderCommand->waitFence();
		}
	}

	reflection.multiLayerTex = textureFactory->Create
	(
		textureFactory->getBuilder()->initProperty()
		->withWidthHeight(size, size)
		->withImageType(VK_IMAGE_TYPE_2D)
		->withImageFlag(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		->withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		->withTiling(VK_IMAGE_TILING_OPTIMAL)
		->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
		->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		->withLayerCount(CUBEMAP_LAYER)
		->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		->withFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
		->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
		->withTargetLayer(0, CUBEMAP_LAYER)
		->withTargetMipmapLevel(0, static_cast<uint32_t>(mipmapLevelSize.size()))
		->addView()
		->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
		->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
		->withMagFilter(VK_FILTER_LINEAR)
		->withMinFilter(VK_FILTER_LINEAR)
		->Build()
	);

	std::shared_ptr<CommandBuffer> copyCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);

	copyCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setFence(commandFactory->createFence());

	for (int i = 0; i < mipmapLevelSize.size(); i++)
	{
		copyCommand->recordBegin();

		for (int j = 0; j < CUBEMAP_LAYER; j++)
		{
			textureFactory->getCopy()->Copy
			(
				textureFactory->getCopy()->initProperty()
				->withAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
				->withSrcLayerRange(0, 1)
				->withDstLayerRange(j, 1)
				->withSrcMipmapLevel(0)
				->withDstMipmapLevel(i)
				->withSize(mipmapLevelSize[i], mipmapLevelSize[i])
				->withCommandBuffer(copyCommand)
				->withSrcTexture(multiLayerTex[i * CUBEMAP_LAYER + j])
				->withDstTexture(reflection.multiLayerTex)
				->Build()
			);
		}

		copyCommand->recordEnd();

		copyCommand->Submit(vulkanCore->getGraphicsQueue());

		copyCommand->waitFence();
	}

	std::shared_ptr<CommandBuffer> transCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);
	transCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setFence(commandFactory->createFence());

	transCommand->recordBegin();

	textureFactory->getBuilder()->transitionImageLayout
	(
		reflection.multiLayerTex->image,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		static_cast<uint32_t>(mipmapLevelSize.size()),
		CUBEMAP_LAYER,
		transCommand
	);

	transCommand->recordEnd();

	transCommand->Submit(vulkanCore->getGraphicsQueue());

	transCommand->waitFence();
}

void SkyDomeBuilder::createBRDF(const std::shared_ptr<SkyDome> skydome, const std::shared_ptr<Texture> srcTexture
	, const std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER>& uniform)
{
	BRDFMap& brdf = skydome->brdf;

	const TextureProperty texProp = srcTexture->property;
	const VkExtent3D extent = srcTexture->property.image.info.extent;
	const uint32_t size = std::min(extent.width, extent.height);

	//1つのレイヤーを持つテクスチャを作成
	brdf.texture = textureFactory->Create
	(
		textureFactory->getBuilder()->initProperty()
		->withWidthHeight(size, size, 1)
		->withImageType(VK_IMAGE_TYPE_2D)
		->withUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		->withTiling(VK_IMAGE_TILING_OPTIMAL)
		->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
		->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		->withLayerCount(1)
		->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
		->withViewType(VK_IMAGE_VIEW_TYPE_2D)
		->withTargetLayer(0, 1)
		->addView()
		->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
		->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
		->withMagFilter(VK_FILTER_LINEAR)
		->withMinFilter(VK_FILTER_LINEAR)
		->Build()
	);

	//上で作ったテクスチャの各レイヤーをレンダーターゲットとして
	//フレームバッファを作る
	std::shared_ptr<FrameBuffer> frameBuffer = frameBufferFactory->Create
	(
		frameBufferFactory->getBuilder()->initProperty()
		->withLayerCount(1)
		->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_BRDF))
		->withWidthHeight(size, size)
		->addViewAttachment(brdf.texture)
		->Build()
	);

	//各フレームバッファにレンダリングしていく
	std::shared_ptr<CommandBuffer> renderCommand = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandFactory);
	renderCommand->setCommandBufffer(commandFactory->createCommandBuffer(1))
		->setFence(commandFactory->createFence());

	renderCommand->recordBegin();

	const RenderProperty renderProp = render->initProperty()
		->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_BRDF))
		->withFrameBuffer(frameBuffer)
		->withCommandBuffer(renderCommand)
		->withRenderArea(size, size)
		->withClearDepth(1.0f)
		->withClearStencil(0)
		->Build();

	render->RenderStart(renderProp);

	std::shared_ptr<GltfModel> gltfModel = modelFactory->GetModel(cubemapModelID);
	if (!gltfModel)
	{
		throw std::runtime_error("GameManager::std::shared_ptr<Render>::gltfModel is nullptr");
	}

	vkCmdBindPipeline(renderCommand->getCommand(), VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineFactory->Create(PipelinePattern::CALC_IBL_BRDF)->pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(renderCommand->getCommand(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { extent.width,extent.height };
	vkCmdSetScissor(renderCommand->getCommand(), 0, 1, &scissor);

	std::array<VkDescriptorSet, 1> descriptorSets;

	VkDeviceSize offsets[] = { 0 };

	for (int i = 0; i < gltfModel->nodes.size(); i++)
	{
		const Mesh& mesh = gltfModel->nodes[i].mesh;

		if (mesh.vertices.size() != 0)
		{
			vkCmdBindVertexBuffers(renderCommand->getCommand(), 0, 1, &gltfModel->getVertBuffer(mesh.meshIndex)->buffer, offsets);

			vkCmdBindIndexBuffer(renderCommand->getCommand(), gltfModel->getIndeBuffer(mesh.meshIndex)->buffer, 0, VK_INDEX_TYPE_UINT32);

			for (const auto& primitive : mesh.primitives)
			{
				descriptorSets[0] = uniform[0]->descriptorSet;

				vkCmdDrawIndexed(renderCommand->getCommand(), primitive.indexCount, 1, primitive.firstIndex, 0, 0);
			}
		}
	}

	render->RenderEnd(renderProp);

	renderCommand->recordEnd();

	renderCommand->Submit(vulkanCore->getGraphicsQueue());

	renderCommand->waitFence();
}

std::shared_ptr<SkyDome> SkyDomeBuilder::Create(const SkyDomeProperty& prop)
{
	std::shared_ptr<SkyDome> skydome = std::make_shared<SkyDome>();

	//元となる画像ファイルのリソースを用意する
	std::shared_ptr<Texture> srcTexture = textureFactory->Create
	(
		prop.srcImagePath,
		TexturePattern::CALC_CUBEMAP
	);

	//マップ計算用のバッファ
	std::array<std::shared_ptr<GpuBuffer>, CUBEMAP_LAYER> uniform;
	std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER> uniformDescriptorSet;
	for (int i = 0; i < CUBEMAP_LAYER; i++)
	{
		uniform[i] = bufferFactory->Create
		(
			sizeof(CameraUniform),
			BufferUsage::UNIFORM,
			BufferTransferType::NONE
		);

		uniformDescriptorSet[i] = descriptorSetFactory->Create
		(
			descriptorSetFactory->getBuilder()->initProperty()
			->withDescriptorSetCount(1)
			->withDescriptorSetLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
			->withBindingBuffer(0)
			->withBuffer(uniform[i])
			->withRange(sizeof(CameraUniform))
			->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			->addBufferInfo()
			->Build()
		);

		switchView(uniformDescriptorSet[i], i);
	}

	std::shared_ptr<DescriptorSet> srcTexDescriptorSet = descriptorSetFactory->Create
	(
		descriptorSetFactory->getBuilder()->initProperty()
		->withDescriptorSetCount(1)
		->withDescriptorSetLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG))
		->withBindingImage(0)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->withTexture(srcTexture)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->addImageInfo()
		->Build()
	);

	//背景のマップを作成
	createBackGround(skydome, srcTexture, uniformDescriptorSet, srcTexDescriptorSet);

	std::cout << "BackGround" << std::endl;

	std::shared_ptr<DescriptorSet> backGroundDesc = descriptorSetFactory->Create
	(
		descriptorSetFactory->getBuilder()->initProperty()
		->withBindingImage(0)
		->withTexture(skydome->backGround.multiLayerTex)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->withDescriptorSetCount(1)
		->withDescriptorSetLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG))
		->addImageInfo()
		->Build()
	);

	//ディフーズのマップを作成
	createDiffuse(skydome, srcTexture, uniformDescriptorSet, backGroundDesc);

	std::cout << "Diffuse Finish" << std::endl;

	//スペキュラーの反射マップを作成
	createReflection(skydome, srcTexture, uniformDescriptorSet, backGroundDesc);

	std::cout << "Reflection Finish" << std::endl;

	textureFactory->getCopy()->initProperty();

	//スペキュラーのBRDFマップを作成
	createBRDF(skydome, srcTexture, uniformDescriptorSet);

	skydome->descriptorSet = descriptorSetFactory->Create
	(
		descriptorSetFactory->getBuilder()->initProperty()
		->withDescriptorSetCount(1)
		->withDescriptorSetLayout(layoutFactory->Create(LayoutPattern::IBL))
		->withBindingImage(0)
		->withTexture(skydome->backGround.multiLayerTex)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->addImageInfo()
		->withBindingImage(1)
		->withTexture(skydome->diffuse.multiLayerTex)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->addImageInfo()
		->withBindingImage(2)
		->withTexture(skydome->reflection.multiLayerTex)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->addImageInfo()
		->withBindingImage(3)
		->withTexture(skydome->brdf.texture)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->addImageInfo()
		->Build()
	);

	return skydome;
}