#include"GpuPipelineBuilder.h"

GpuPipelineBuilder::GpuPipelineBuilder(VkDevice& d, std::shared_ptr<ShaderFactory> sf)
{
	device = d;

	shaderFactory = sf;

	//プロパティの初期化
	property.initProperty();
}

void GpuPipelineBuilder::Create(PipelineProperty& p)
{
	if (!property.computeShader)
	{
		property.stages.resize(2);

		property.stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		property.stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		property.stages[0].module = property.vertexShader->module;
		property.stages[0].pName = "main";

		property.stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		property.stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		property.stages[1].module = property.fragmentShader->module;
		property.stages[1].pName = "main";
	}
	else if(property.vertexShader && property.fragmentShader)
	{
		property.stages.resize(1);

		property.stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		property.stages[0].stage = VK_SHADER_STAGE_COMPUTE_BIT;
		property.stages[0].module = property.computeShader->module;
		property.stages[0].pName = "main";
	}
	else
	{
		throw std::runtime_error("GpuPipelineBuilder : Create() : 不正なシェーダパスの設定");
	}

	property.vertexInputState.vertexBindingDescriptionCount = 1;
	property.vertexInputState.pVertexBindingDescriptions = &property.bindingDescription;
	property.vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(property.attributeDescriptions.size());
	property.vertexInputState.pVertexAttributeDescriptions = property.attributeDescriptions.data();

	property.colorBlendState.attachmentCount = static_cast<uint32_t>(property.colorBlendStateArray.size());
	property.colorBlendState.pAttachments = property.colorBlendStateArray.data();

	property.dynamicState.dynamicStateCount = static_cast<uint32_t>(property.dynamicStateArray.size());
	property.dynamicState.pDynamicStates = property.dynamicStateArray.data();

	p = property;

	initProperty();
}

//パイプラインの設定の初期化
void GpuPipelineBuilder::initProperty()
{
	property.initProperty();
}

//パイプラインレイアウトを設定する
GpuPipelineBuilder GpuPipelineBuilder::withPipelineLayout(const std::shared_ptr<PipelineLayout>& pLayout)
{
	property.pLayout = pLayout;

	return *this;
}

//レンダーパスを設定する
GpuPipeilneBuilder GpuPipelineBuilder::withRenderPass(const std::shared_ptr<RenderPass>& renderPass)
{
	property.renderPass = renderPass;

	return *this;
}

//頂点シェーダパスを設定
GpuPipelineBuilder GpuPipelineBuilder::withVertexShader(const std::string& path)
{
	if (!property.computeShader)
	{
		throw std::runtime_error("GpuPipelineBuilder : すでにコンピュートシェーダが登録されています");
	}

	property.vertexShader = shaderFactory->Create(path);

	return *this;
}
//フラグメントシェーダパスを設定
GpuPipelineBuilder GpuPipelineBuilder::withFragmentShader(const std::string& path)
{
	if (!property.computeShader)
	{
		throw std::runtime_error("GpuPipelineBuilder : すでにコンピュートシェーダが登録されています");
	}

	property.fragmentShader = shaderFactory->Create(path);

	return *this;
}
//コンピュートシェーダ
GpuPipelineBuilder GpuPipelineBuilder::withComputeShader(const std::string& path)
{
	if (!property.vertexShader || !property.fragmentShader)
	{
		throw std::runtime_error("GpuPipelineBuilder : すでに頂点かフラグメントシェーダが登録されています");
	}

	property.computeShader = shaderFactory->Create(path);

	return *this;
}

//頂点データの読み取り方を設定
GpuPipelineBuilder GpuPipelineBuilder::withVertexInput(const uint32_t& binding, const VkVertexInputRate& rate
	, const uint32_t& stride)
{
	property.bindingDescription.binding = binding;
	property.bindingDescription.inputRate = rate;
	property.bindingDescription.stride = stride;

	return *this;
}

//頂点アトリビュートの追加
GpuPipelineBuilder GpuPipelineBuilder::addVertexInputAttrib(const uint32_t& location, const uint32_t& binding,
	const VkFormat& format, const uint32_t& offset)
{
	VkVertexInputAttributeDescription attribute{};
	attribute.location = location;
	attribute.binding = binding;
	attribute.format = format;
	attribute.offset = offset;

	property.attributeDescriptions.push_back(attribute);

	return *this;
}

//プリミティブの設定
GpuPipelineBuilder GpuPipelineBuilder::withTopology(const VkPrimitiveTopology& topology)
{
	property.inputAssemblyState.topology = topology;

	return *this;
}


//プリミティブの描画方法を設定
GpuPipelineBuilder GpuPipelineBuilder::withPolygonMode(const VkPolygonMode& mode)
{
	property.rasterizationState.polygonMode = mode;

	return *this;
}
//ラインの太さを設定
GpuPipelineBuilder GpuPipelineBuilder::withLineWidth(const float& width)
{
	property.rasterizationState.lineWidth = width;

	return *this;
}
//カリングモードの設定
GpuPipelineBuilder GpuPipelineBuilder::withCulModel(const VkCullModeFlags& mode)
{
	property.rasterizationState.cullMode = mode;

	return *this;
}
//ポリゴンの表裏判定を右回りか左回りに設定
GpuPipelineBuilder GpuPipelineBuilder::withFrontFace(const VkFrontFace& face)
{
	property.rasterizationState.frontFace = face;

	return *this;
}
//デプスバイアスを設定する
GpuPipelineBuilder GpuPipelineBuilder::enableDepthBias(const bool& isBias)
{
	property.rasterizationState.depthBiasEnable = static_cast<VkBool32>(isBias);

	return *this;
}


//マルチサンプリングシェーディングを設定
GpuPipelineBuilder GpuPipelineBuilder::enableMultiSampleShading(const bool& mode)
{
	property.multisampleState.sampleShadingEnable = static_cast<VkBool32>(mode);

	return *this;
}
//最低のサンプリングシェーディングポイントの数を設定
GpuPipelineBuilder GpuPipelineBuilder::withMinSampleShading(const float& min)
{
	property.multisampleState.minSampleShading = min;

	return *this;
}
//マルチサンプリングを行う際のサンプル数を設定
GpuPipelineBuilder GpuPipelineBuilder::withRansterizationSamples(const VkSampleCountFlagBits& flag)
{
	property.multisampleState.rasterizationSamples = flag;

	return *this;
}


//デプステストを設定する
GpuPipelineBuilder GpuPipelineBuilder::enableDepthTest(const bool& isEnable)
{
	property.depthStencilState.depthTestEnable = static_cast<VkBool32>(isEnable);

	return *this;
}
//zバッファへの書き込みを設定する
GpuPipelineBuilder GpuPipelineBuilder::enableDepthWrite(const bool& isWrite)
{
	property.depthStencilState.depthWriteEnable = static_cast<VkBool32>(isWrite);

	return *this;
}
//z値の比較の仕方を設定する
GpuPipelineBuilder GpuPipelineBuilder::withDepthCompare(const VkCompareOp& compare)
{
	property.depthStencilState.depthCompareOp = compare;

	return *this;
}
//z値の範囲を指定し、それに満たない頂点は破棄する
GpuPipelineBuilder GpuPipelineBuilder::enableDepthBoundsTest(const float& min, const float& max)
{
	property.depthStencilState.depthBoundsTestEnable = VK_TRUE;
	property.depthStencilState.minDepthBounds = min;
	property.depthStencilState.maxDepthBounds = max;

	return *this;
}
//ステンシルテストを設定する
GpuPipelineBuilder GpuPipelineBuilder::enableStencilTest(const VkStencilOpState& front, const VkStencilOpState& back)
{
	property.depthStencilState.stencilTestEnable = VK_TRUE;
	property.depthStencilState.front = front;
	property.depthStencilState.back = back;

	return *this;
}


//ロジック演算を設定
GpuPipelineBuilder GpuPipelineBuilder::withLogicOp(const VkLogicOp& logic)
{
	property.colorBlendState.logicOpEnable = VK_TRUE;
	property.colorBlendState.logicOp = logic;

	return *this;
}
//アタッチメントを追加
GpuPipelineBuilder GpuPipelineBuilder::addColoarAttachment(const VkPipelineColorBlendAttachmentState& attachment)
{
	property.colorBlendStateArray.push_back(attachment);

	return *this;
}
//ブレンド定数を設定
GpuPipelineBuilder GpuPipelineBuilder::withBlendConstant(const float& r, const float& g, const float& b, const float& a)
{
	property.colorBlendState.blendConstants[0] = r;
	property.colorBlendState.blendConstants[1] = g;
	property.colorBlendState.blendConstants[2] = b;
	property.colorBlendState.blendConstants[3] = a;

	return *this;
}

//動的に変更するステートを積み上げる
GpuPipelineBuilder GpuPipelineBuilder::addDynamicState(const VkDynamicState& state)
{
	property.dynamicStateArray.push_back(state);

	return *this;
}