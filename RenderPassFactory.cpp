#include"RenderPassFactory.h"

RenderPassFactory::RenderPassFactory(std::shared_ptr<VulkanCore> core, std::shared_ptr<RenderPassBuilder> b)
{
	vulkanCore = core;

	device = vulkanCore->getLogicDevice();

	builder = b;

	frameIndex = 1;
}

//パターンからプロパティを作る
RenderPassProperty RenderPassFactory::convertPattern(const RenderPassPattern& pattern)
{
	if (pattern == RenderPassPattern::PBR)
	{
		builder->initProperty();

		builder->withFormat(vulkanCore->getSwapChainFormat())
			.withMultiSamples(vulkanCore->getMaxMsaaSamples())
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addColorAttachment();

		builder->withFormat(VK_FORMAT_D32_SFLOAT_S8_UINT)
			.withMultiSamples(vulkanCore->getMaxMsaaSamples())
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.addDepthStencilAttachment();

		builder->withFormat(vulkanCore->getSwapChainFormat())
			.withMultiSamples(VK_SAMPLE_COUNT_1_BIT)
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.addColorResolveAttachment();

		builder->withSrcSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withDstSubpassIndex(0)
			.withSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.withSrcAccessMask(0)
			.withDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.withDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.addDependency()
			.addSubpass();

		return builder->Build();
	}
	else if (pattern == RenderPassPattern::UI)
	{
		builder->initProperty();

		builder->withFormat(VK_FORMAT_R8G8B8A8_UNORM)
			.withMultiSamples(VK_SAMPLE_COUNT_8_BIT)
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addColorAttachment();

		builder->withFormat(VK_FORMAT_D32_SFLOAT)
			.withMultiSamples(VK_SAMPLE_COUNT_1_BIT)
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.addDepthStencilAttachment();

		builder->withFormat(VK_FORMAT_R8G8B8A8_UNORM)
			.withMultiSamples(VK_SAMPLE_COUNT_1_BIT)
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.addColorResolveAttachment();

		builder->withSrcSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withDstSubpassIndex(0)
			.withSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.withSrcAccessMask(0)
			.withDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.withDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.addDependency()
			.addSubpass();

		return builder->Build();
	}
	else if (pattern == RenderPassPattern::CALC_SHADOWMAP)
	{
		builder->initProperty();

		builder->withFormat(VK_FORMAT_D16_UNORM)
			.withMultiSamples(VK_SAMPLE_COUNT_1_BIT)
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			.addDepthStencilAttachment();

		builder->withSrcSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withDstSubpassIndex(0)
			.withSrcStageMask(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			.withDstStageMask(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.withSrcAccessMask(VK_ACCESS_SHADER_READ_BIT)
			.withDstAccessMask(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.withFlag(VK_DEPENDENCY_BY_REGION_BIT)
			.addDependency();

		builder->withSrcSubpassIndex(0)
			.withDstSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withSrcStageMask(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
			.withDstStageMask(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			.withSrcAccessMask(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.withDstAccessMask(VK_ACCESS_SHADER_READ_BIT)
			.withFlag(VK_DEPENDENCY_BY_REGION_BIT)
			.addDependency();

		builder->addSubpass();

		return builder->Build();
	}
	else if (pattern == RenderPassPattern::CALC_CUBEMAP)
	{
		builder->initProperty();

		builder->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.withMultiSamples(VK_SAMPLE_COUNT_1_BIT)
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addColorAttachment();

		builder->withSrcSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withDstSubpassIndex(0)
			.withSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.withDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.withSrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.withDstAccessMask(0)
			.withFlag(VK_DEPENDENCY_BY_REGION_BIT)
			.addDependency();

		builder->withSrcSubpassIndex(0)
			.withDstSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.withDstStageMask(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			.withSrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.withDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			.withFlag(VK_DEPENDENCY_BY_REGION_BIT)
			.addDependency();

		builder->addSubpass();

		return builder->Build();
	}
	else if (pattern == RenderPassPattern::CALC_IBL)
	{
		//CALC_CUBEMAPと同じ

		builder->initProperty();

		builder->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.withMultiSamples(VK_SAMPLE_COUNT_1_BIT)
			.withColorLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
			.withColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.withStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.withStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addColorAttachment();

		builder->withSrcSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withDstSubpassIndex(0)
			.withSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.withDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.withSrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.withDstAccessMask(0)
			.withFlag(VK_DEPENDENCY_BY_REGION_BIT)
			.addDependency();

		builder->withSrcSubpassIndex(0)
			.withDstSubpassIndex(VK_SUBPASS_EXTERNAL)
			.withSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.withDstStageMask(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			.withSrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.withDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			.withFlag(VK_DEPENDENCY_BY_REGION_BIT)
			.addDependency();

		builder->addSubpass();

		return builder->Build();
	}
}

//レンダーパスを作成する
std::shared_ptr<RenderPass> RenderPassFactory::Create(const RenderPassProperty& property)
{
	std::shared_ptr<RenderPass> renderPass = std::make_shared<RenderPass>(shared_from_this());

	//VkRenderPassを作成する
	builder->Create(property, renderPass->renderPass);

	//ハッシュ値を計算する
	renderPass->hashKey = hash(property);

	//ストレージに記録する
	renderPassStorage[property] = renderPass;

	return renderPass;
}

//レンダーパスを作成する
std::shared_ptr<RenderPass> RenderPassFactory::Create(const RenderPassPattern& pattern)
{
	std::shared_ptr<RenderPass> renderPass = std::make_shared<RenderPass>(shared_from_this());

	RenderPassProperty property = convertPattern(pattern);

	//VkRenderPassを作成する
	builder->Create(property, renderPass->renderPass);

	//ハッシュ値を計算する
	renderPass->hashKey = hash(property);

	//ストレージに記録する
	renderPassStorage[property] = renderPass;

	return renderPass;
}

//遅延破棄リストにリソースを追加する
void RenderPassFactory::addDefferedDestruct(VkRenderPass& renderPass)
{
	destructList[frameIndex].push_back(renderPass);
}

//リソースを破棄する
void RenderPassFactory::resourceDestruct()
{
	frameIndex = (frameIndex == 0) ? 1 : 0;

	for (auto& pass : destructList[frameIndex])
	{
		vkDestroyRenderPass(device, pass, nullptr);
	}

	destructList[frameIndex].clear();
}