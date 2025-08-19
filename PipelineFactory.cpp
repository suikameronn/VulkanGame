#include"PipelineFactory.h"

PipelineFactory::PipelineFactory(std::shared_ptr<VulkanCore> core, std::shared_ptr<PipelineLayoutFactory> f
    , std::shared_ptr<ShaderFactory> sf, std::shared_ptr<PipelineBuilder> b
    , std::shared_ptr<RenderPassFactory> r)
{
    vulkanCore = core;

    device = vulkanCore->getLogicDevice();

    pLayoutFactory = f;

    shaderFactory = sf;

    renderPassFactory = r;

    builder = b;

    frameIndex = 1;
}

PipelineFactory::~PipelineFactory()
{
    for (auto& itr : pipelineStorage)
    {
        if (!itr.second.expired())
        {
            //本来はここで破棄されるものはないはず
            itr.second.lock().reset();
        }
    }

    for (int i = 0; i < 2; i++)
    {
        resourceDestruct();
        resourceDestruct();
    }

#ifdef _DEBUG
    std::cout << "PipelineFactory :: デストラクタ" << std::endl;
#endif
}

PipelineProperty PipelineFactory::convertPattern(const PipelinePattern& pattern)
{
    if (pattern == PipelinePattern::PBR)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/vert.spv")
            ->withFragmentShader("shaders/frag.spv")
            ->withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            ->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            ->addVertexInputAttrib(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
            ->addVertexInputAttrib(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord0))
            ->addVertexInputAttrib(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord1))
            ->addVertexInputAttrib(4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
            ->addVertexInputAttrib(5, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(Vertex, boneID1))
            ->addVertexInputAttrib(6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight1))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withLineWidth(1.0f)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withCullMode(VK_CULL_MODE_BACK_BIT)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableDepthBias(VK_FALSE)
            ->enableMultiSampleShading(VK_TRUE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(vulkanCore->getMaxMsaaSamples())
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS)
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::PBR))
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::PBR))
            ->Build();
    }
    else if (pattern == PipelinePattern::UI)
    {
        builder->initProperty();

        return builder->withVertexShader("ui.vert.spv")
			->withFragmentShader("ui.frag.spv")
            ->withVertexInput(0,VK_VERTEX_INPUT_RATE_VERTEX,sizeof(Vertex2D))
			->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex2D, pos))
			->addVertexInputAttrib(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2D, uv))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withLineWidth(1.0f)
            ->withCullMode(VK_CULL_MODE_NONE)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableMultiSampleShading(VK_TRUE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(VK_SAMPLE_COUNT_8_BIT)
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS)
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::UI))
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::UI))
            ->Build();
            
    }
    else if (pattern == PipelinePattern::CUBEMAP)
    {
        builder->initProperty();

       return builder->withVertexShader("shaders/cubemap.vert.spv")
            ->withFragmentShader("shaders/cubemap.frag.spv")
            ->withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            ->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withLineWidth(1.0f)
            ->withCullMode(VK_CULL_MODE_NONE)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableMultiSampleShading(VK_TRUE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_CUBEMAP))
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_CUBEMAP))
            ->Build();
    }
    else if (pattern == PipelinePattern::CALC_SHADOWMAP)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/shadowMapping.vert.spv")
            ->withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            ->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            ->addVertexInputAttrib(1, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(Vertex, boneID1))
            ->addVertexInputAttrib(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight1))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withLineWidth(1.0f)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withCullMode(VK_CULL_MODE_BACK_BIT)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableDepthBias(VK_TRUE)
            ->enableMultiSampleShading(VK_TRUE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->addDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_SHADOWMAP))
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
            ->Build();
    }
    else if (pattern == PipelinePattern::CALC_CUBEMAP)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/calcCubemap.vert.spv")
            ->withFragmentShader("shaders/calcCubemap.frag.spv")
            ->withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            ->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withLineWidth(1.0f)
            ->withCullMode(VK_CULL_MODE_NONE)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableMultiSampleShading(VK_FALSE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_CUBEMAP))
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_CUBEMAP))
            ->Build();

    }
    else if (pattern == PipelinePattern::CALC_IBL_DIFFUSE)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/calcIBL.vert.spv")
            ->withFragmentShader("shaders/calcDiffuse.frag.spv")
            ->withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            ->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            ->addVertexInputAttrib(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, texCoord0))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withLineWidth(1.0f)
            ->withCullMode(VK_CULL_MODE_BACK_BIT)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableMultiSampleShading(VK_TRUE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_DIFFUSE_SPECULAR))
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_IBL_DIFFUSE))
            ->Build();
    }
    else if (pattern == PipelinePattern::CALC_IBL_SPECULAR)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/calcIBL.vert.spv")
            ->withFragmentShader("shaders/calcSpecularReflection.frag.spv")
            ->withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            ->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            ->addVertexInputAttrib(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, texCoord0))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withLineWidth(1.0f)
            ->withCullMode(VK_CULL_MODE_BACK_BIT)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableMultiSampleShading(VK_TRUE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_DIFFUSE_SPECULAR))
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_IBL_SPECULAR))
            ->Build();
    }
    else if (pattern == PipelinePattern::CALC_IBL_BRDF)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/calcBRDF.vert.spv")
            ->withFragmentShader("shaders/calcSpecularBRDF.frag.spv")
            ->withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            ->addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            ->withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            ->withPolygonMode(VK_POLYGON_MODE_FILL)
            ->withLineWidth(1.0f)
            ->withCullMode(VK_CULL_MODE_BACK_BIT)
            ->withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            ->enableMultiSampleShading(VK_TRUE)
            ->withMinSampleShading(0.2f)
            ->withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            ->withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            ->withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            ->withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            ->addColoarAttachment()
            ->withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            ->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            ->addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            ->enableDepthTest(VK_TRUE)
            ->enableDepthWrite(VK_TRUE)
            ->withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            ->withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_IBL_BRDF))
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_IBL_BRDF))
            ->Build();
    }
    else if (pattern == PipelinePattern::RAYCAST)
    {

        builder->initProperty();

		return builder->withComputeShader("shaders/raycast->comp->spv")
            ->withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::RAYCAST))
            ->Build();
    }
}

//パイプラインを作成する
std::shared_ptr<Pipeline> PipelineFactory::Create(const PipelinePattern& pattern)
{
    PipelineProperty property = convertPattern(pattern);

    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(shared_from_this());

    builder->Create(property, pipeline->pipeline);

    return pipeline;
}

//パイプラインを作成する
std::shared_ptr<Pipeline> PipelineFactory::Create(const PipelineProperty& property)
{
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(shared_from_this());

    builder->Create(property, pipeline->pipeline);

    return pipeline;
}

//遅延破棄リストにリソースを追加する
void PipelineFactory::addDefferedDestruct(VkPipeline& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//リソースを破棄する
void PipelineFactory::resourceDestruct()
{
    frameIndex = (frameIndex == 0) ? 1 : 0;

	for (auto& resource : destructList[frameIndex])
	{
		vkDestroyPipeline(device, resource, nullptr);
	}

    destructList[frameIndex].clear();
}