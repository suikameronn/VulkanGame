#include"GpuPipelineFactory.h"

GpuPipelineFactory::GpuPipelineFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutFactory> f
    , std::shared_ptr<Shader> sf, std::shared_ptr<GpuPipelineBuilder> b)
{
    device = d;

    pLayoutFactory = f;

    shaderFactory = sf;

    builder = b;

    frameIndex = 1;
}

PipelineProperty GpuPipelineFactory::convertPattern(const PipelinePattern& pattern)
{

}

//パイプラインを作成する
std::shared_ptr<Pipeline> GpuPipelineFactory::Create(const PipelinePattern& pattern)
{

}

//パイプラインを作成する
std::shared_ptr<Pipeline> GpuPipelineFactory::Create(const PipelineProperty& property)
{
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(shared_from_this());

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(property.stages.size());
    pipelineInfo.pStages = property.stages.data();
    pipelineInfo.pVertexInputState = &property.vertexInputState;
    pipelineInfo.pInputAssemblyState = &property.inputAssemblyState;
    pipelineInfo.pViewportState = &property.viewportState;
    pipelineInfo.pRasterizationState = &property.rasterizationState;
    pipelineInfo.pMultisampleState = &property.multisampleState;
    pipelineInfo.pColorBlendState = &property.colorBlendState;
    pipelineInfo.pDynamicState = &property.dynamicState;
    pipelineInfo.layout = property.pLayout->pLayout;
    pipelineInfo.renderPass = property.renderPass->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &property.depthStencilState;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return pipeline;
}

//遅延破棄リストにリソースを追加する
void GpuPipelineFactory::addDefferedDestruct(VkPipeline& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//リソースを破棄する
void GpuPipelineFactory::resourceDestruct()
{
	for (auto& resource : destructList[frameIndex])
	{
		vkDestroyPipeline(device, resource, nullptr);
	}

	frameIndex = (frameIndex == 0) ? 1 : 0;
}