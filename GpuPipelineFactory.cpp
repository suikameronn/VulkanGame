#include"GpuPipelineFactory.h"

GpuPipelineFactory::GpuPipelineFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutFactory> f
    , std::shared_ptr<Shader> sf, std::shared_ptr<GpuPipelineBuilder> b
    , std::shared_ptr<GpuRenderPassFactory> r)
{
    device = d;

    pLayoutFactory = f;

    shaderFactory = sf;

    renderPassFactory = r;

    builder = b;

    frameIndex = 1;
}

PipelineProperty GpuPipelineFactory::convertPattern(const PipelinePattern& pattern)
{
    if (pattern == PipelinePattern::PBR)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/vert.spv")
            .withFragmentShader("shaders/frag.spv")
            .withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord0))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord1))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, boneID1))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight1))
            .withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withLineWidth(1.0f)
            .withPolygonMode(VK_POLYGON_MODE_FILL)
            .withCullMode(VK_CULL_MODE_BACK_BIT)
            .withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .enableDepthBias(VK_FALSE)
            .enableMultiSampleShading(VK_TRUE)
            .withMinSampleShading(0.2f)
            .withRansterizationSamples(VK_SAMPLE_COUNT_8_BIT)
            .withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            .withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            .withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            .addColoarAttachment()
            .withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            .enableDepthTest(VK_TRUE)
            .enableDepthWrite(VK_TRUE)
            .withDepthCompare(VK_COMPARE_OP_LESS)
            .withRenderPass(renderPassFactory->Create(RenderPassPattern::PBR))
            .withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::PBR))
            .Build();
    }
    else if (pattern == PipelinePattern::UI)
    {

    }
    else if (pattern == PipelinePattern::CUBEMAP)
    {
        builder->initProperty();

       return builder->withVertexShader("shaders/cubemap.vert.spv")
            .withFragmentShader("shaders/cubemap.frag.spv")
            .withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withPolygonMode(VK_POLYGON_MODE_FILL)
            .withLineWidth(1.0f)
            .withCullMode(VK_CULL_MODE_BACK_BIT)
            .withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .enableMultiSampleShading(VK_TRUE)
            .withMinSampleShading(0.2f)
            .withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            .withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            .withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            .withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            .addColoarAttachment()
            .withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            .enableDepthTest(VK_TRUE)
            .enableDepthWrite(VK_TRUE)
            .withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            .withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_CUBEMAP))
            .withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_CUBEMAP))
            .Build();
    }
    else if (pattern == PipelinePattern::CALC_SHADOWMAP)
    {
        return builder->withVertexShader("shaders/shadowMapping.vert.spv")
            .withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .addVertexInputAttrib(1, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(Vertex, boneID1))
            .addVertexInputAttrib(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight1))
            .withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withLineWidth(1.0f)
            .withPolygonMode(VK_POLYGON_MODE_FILL)
            .withCullMode(VK_CULL_MODE_BACK_BIT)
            .withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .enableDepthBias(VK_TRUE)
            .enableMultiSampleShading(VK_TRUE)
            .withMinSampleShading(0.2f)
            .withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            .withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            .withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            .withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            .addColoarAttachment()
            .withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            .addDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS)
            .enableDepthTest(VK_TRUE)
            .enableDepthWrite(VK_TRUE)
            .withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            .withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_SHADOWMAP))
            .withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_SHADOWMAP))
            .Build();
    }
    else if (pattern == PipelinePattern::CALC_CUBEMAP)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/calcCubemap.vert.spv")
            .withFragmentShader("shaders/calcCubemap.frag.spv")
            .withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withPolygonMode(VK_POLYGON_MODE_FILL)
            .withLineWidth(1.0f)
            .withCullMode(VK_CULL_MODE_BACK_BIT)
            .withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .enableMultiSampleShading(VK_TRUE)
            .withMinSampleShading(0.2f)
            .withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            .withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            .withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            .withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            .addColoarAttachment()
            .withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            .enableDepthTest(VK_TRUE)
            .enableDepthWrite(VK_TRUE)
            .withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            .withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_CUBEMAP))
            .withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_CUBEMAP))
            .Build();

    }
    else if (pattern == PipelinePattern::CALC_IBL_DIFFUSE)
    {
        builder->initProperty();

        return builder->withVertexShader("shaders/calcIBL.vert.spv")
            .withFragmentShader("shaders/calcDiffuse.frag.spv")
            .withVertexInput(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex))
            .addVertexInputAttrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .withTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withPolygonMode(VK_POLYGON_MODE_FILL)
            .withLineWidth(1.0f)
            .withCullMode(VK_CULL_MODE_BACK_BIT)
            .withFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .enableMultiSampleShading(VK_TRUE)
            .withMinSampleShading(0.2f)
            .withRansterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            .withColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
            .withColorBlendFactorOp(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                , VK_BLEND_OP_ADD)
            .withAlphaBlendFactorOp(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
                , VK_BLEND_OP_ADD)
            .addColoarAttachment()
            .withBlendConstant(0.0f, 0.0f, 0.0f, 0.0f)
            .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            .enableDepthTest(VK_TRUE)
            .enableDepthWrite(VK_TRUE)
            .withDepthCompare(VK_COMPARE_OP_LESS_OR_EQUAL)
            .withRenderPass(renderPassFactory->Create(RenderPassPattern::CALC_CUBEMAP))
            .withPipelineLayout(pLayoutFactory->Create(PipelineLayoutPattern::CALC_CUBEMAP))
            .Build();
    }
    else if (pattern == PipelinePattern::CALC_IBL_SPECULAR)
    {

    }
    else if (pattern == PipelinePattern::CALC_IBL_BRDF)
    {

    }
    else if (pattern == PipelinePattern::RAYCAST)
    {

    }
}

//パイプラインを作成する
std::shared_ptr<Pipeline> GpuPipelineFactory::Create(const PipelinePattern& pattern)
{
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(shared_from_this());

    PipelineProperty property = convertPattern(pattern);

    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(shared_from_this());

    builder->Create(property, pipeline->pipeline);

    return pipeline;
}

//パイプラインを作成する
std::shared_ptr<Pipeline> GpuPipelineFactory::Create(const PipelineProperty& property)
{
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(shared_from_this());

    builder->Create(property, pipeline->pipeline);

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