#include"PipelineBuilder.h"

PipelineBuilder::PipelineBuilder(VkDevice& d, std::shared_ptr<ShaderFactory> sf)
{
	device = d;

	shaderFactory = sf;

	colorBlendAttachment = VkPipelineColorBlendAttachmentState{};

	//プロパティの初期化
	property.initProperty();
}

PipelineProperty PipelineBuilder::Build()
{
	if (!property.computeShader)
	{
		if (!property.fragmentShader)
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
		else
		{
			property.stages.resize(1);

			property.stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			property.stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			property.stages[0].module = property.vertexShader->module;
			property.stages[0].pName = "main";
		}
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
		throw std::runtime_error("PipelineBuilder : Create() : 不正なシェーダパスの設定");
	}

	property.vertexInputState.vertexBindingDescriptionCount = 1;
	property.vertexInputState.pVertexBindingDescriptions = &property.bindingDescription;
	property.vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(property.attributeDescriptions.size());
	property.vertexInputState.pVertexAttributeDescriptions = property.attributeDescriptions.data();

	property.colorBlendState.attachmentCount = static_cast<uint32_t>(property.colorBlendStateArray.size());
	property.colorBlendState.pAttachments = property.colorBlendStateArray.data();

	property.dynamicState.dynamicStateCount = static_cast<uint32_t>(property.dynamicStateArray.size());
	property.dynamicState.pDynamicStates = property.dynamicStateArray.data();

	PipelineProperty p = property;

	initProperty();

	return p;
}

//VkPipelineを作成
void PipelineBuilder::Create(const PipelineProperty& p, VkPipeline& pipeline)
{
	if (p.computeShader)
	{
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(p.stages.size());
		pipelineInfo.pStages = p.stages.data();
		pipelineInfo.pVertexInputState = &p.vertexInputState;
		pipelineInfo.pInputAssemblyState = &p.inputAssemblyState;
		pipelineInfo.pViewportState = &p.viewportState;
		pipelineInfo.pRasterizationState = &p.rasterizationState;
		pipelineInfo.pMultisampleState = &p.multisampleState;
		pipelineInfo.pColorBlendState = &p.colorBlendState;
		pipelineInfo.pDynamicState = &p.dynamicState;
		pipelineInfo.layout = p.pLayout->pLayout;
		pipelineInfo.renderPass = p.renderPass->renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &p.depthStencilState;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}
	else
	{
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = p.computeShader->module;
		computeShaderStageInfo.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = p.pLayout->pLayout;
		pipelineInfo.stage = computeShaderStageInfo;

		if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline!");
		}
	}
}

//パイプラインの設定の初期化
void PipelineBuilder::initProperty()
{
	property.initProperty();
}

//パイプラインレイアウトを設定する
PipelineBuilder PipelineBuilder::withPipelineLayout(const std::shared_ptr<PipelineLayout>& pLayout)
{
	property.pLayout = pLayout;

	return *this;
}

//レンダーパスを設定する
PipelineBuilder PipelineBuilder::withRenderPass(const std::shared_ptr<RenderPass>& renderPass)
{
	property.renderPass = renderPass;

	return *this;
}

//頂点シェーダパスを設定
PipelineBuilder PipelineBuilder::withVertexShader(const std::string& path)
{
	if (!property.computeShader)
	{
		throw std::runtime_error("PipelineBuilder : すでにコンピュートシェーダが登録されています");
	}

	property.vertexShader = shaderFactory->Create(path);

	return *this;
}
//フラグメントシェーダパスを設定
PipelineBuilder PipelineBuilder::withFragmentShader(const std::string& path)
{
	if (!property.computeShader)
	{
		throw std::runtime_error("PipelineBuilder : すでにコンピュートシェーダが登録されています");
	}

	property.fragmentShader = shaderFactory->Create(path);

	return *this;
}
//コンピュートシェーダ
PipelineBuilder PipelineBuilder::withComputeShader(const std::string& path)
{
	if (!property.vertexShader || !property.fragmentShader)
	{
		throw std::runtime_error("PipelineBuilder : すでに頂点かフラグメントシェーダが登録されています");
	}

	property.computeShader = shaderFactory->Create(path);

	return *this;
}

//頂点データの読み取り方を設定
PipelineBuilder PipelineBuilder::withVertexInput(const uint32_t& binding, const VkVertexInputRate& rate
	, const uint32_t& stride)
{
	property.bindingDescription.binding = binding;
	property.bindingDescription.inputRate = rate;
	property.bindingDescription.stride = stride;

	return *this;
}

//頂点アトリビュートの追加
PipelineBuilder PipelineBuilder::addVertexInputAttrib(const uint32_t& location, const uint32_t& binding,
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
PipelineBuilder PipelineBuilder::withTopology(const VkPrimitiveTopology& topology)
{
	property.inputAssemblyState.topology = topology;

	return *this;
}


//プリミティブの描画方法を設定
PipelineBuilder PipelineBuilder::withPolygonMode(const VkPolygonMode& mode)
{
	property.rasterizationState.polygonMode = mode;

	return *this;
}
//ラインの太さを設定
PipelineBuilder PipelineBuilder::withLineWidth(const float& width)
{
	property.rasterizationState.lineWidth = width;

	return *this;
}
//カリングモードの設定
PipelineBuilder PipelineBuilder::withCullMode(const VkCullModeFlags& mode)
{
	property.rasterizationState.cullMode = mode;

	return *this;
}
//ポリゴンの表裏判定を右回りか左回りに設定
PipelineBuilder PipelineBuilder::withFrontFace(const VkFrontFace& face)
{
	property.rasterizationState.frontFace = face;

	return *this;
}
//デプスバイアスを設定する
PipelineBuilder PipelineBuilder::enableDepthBias(const bool& isBias)
{
	property.rasterizationState.depthBiasEnable = static_cast<VkBool32>(isBias);

	return *this;
}


//マルチサンプリングシェーディングを設定
PipelineBuilder PipelineBuilder::enableMultiSampleShading(const bool& mode)
{
	property.multisampleState.sampleShadingEnable = static_cast<VkBool32>(mode);

	return *this;
}
//最低のサンプリングシェーディングポイントの数を設定
PipelineBuilder PipelineBuilder::withMinSampleShading(const float& min)
{
	property.multisampleState.minSampleShading = min;

	return *this;
}
//マルチサンプリングを行う際のサンプル数を設定
PipelineBuilder PipelineBuilder::withRansterizationSamples(const VkSampleCountFlagBits& flag)
{
	property.multisampleState.rasterizationSamples = flag;

	return *this;
}


//デプステストを設定する
PipelineBuilder PipelineBuilder::enableDepthTest(const bool& isEnable)
{
	property.depthStencilState.depthTestEnable = static_cast<VkBool32>(isEnable);

	return *this;
}
//zバッファへの書き込みを設定する
PipelineBuilder PipelineBuilder::enableDepthWrite(const bool& isWrite)
{
	property.depthStencilState.depthWriteEnable = static_cast<VkBool32>(isWrite);

	return *this;
}
//z値の比較の仕方を設定する
PipelineBuilder PipelineBuilder::withDepthCompare(const VkCompareOp& compare)
{
	property.depthStencilState.depthCompareOp = compare;

	return *this;
}
//z値の範囲を指定し、それに満たない頂点は破棄する
PipelineBuilder PipelineBuilder::enableDepthBoundsTest(const float& min, const float& max)
{
	property.depthStencilState.depthBoundsTestEnable = VK_TRUE;
	property.depthStencilState.minDepthBounds = min;
	property.depthStencilState.maxDepthBounds = max;

	return *this;
}
//ステンシルテストを設定する
PipelineBuilder PipelineBuilder::enableStencilTest(const VkStencilOpState& front, const VkStencilOpState& back)
{
	property.depthStencilState.stencilTestEnable = VK_TRUE;
	property.depthStencilState.front = front;
	property.depthStencilState.back = back;

	return *this;
}


//ロジック演算を設定
PipelineBuilder PipelineBuilder::withLogicOp(const VkLogicOp& logic)
{
	property.colorBlendState.logicOpEnable = VK_TRUE;
	property.colorBlendState.logicOp = logic;

	return *this;
}

//カラーブレンドアタッチメントの書き込みできる色を設定する
PipelineBuilder PipelineBuilder::withColorWriteMask(const VkColorComponentFlags& flag)
{
	colorBlendAttachment.colorWriteMask = flag;

	return *this;
}

//色のブレンドの仕方を設定する
PipelineBuilder PipelineBuilder::withColorBlendFactorOp(const VkBlendFactor& src, const VkBlendFactor& dst, const VkBlendOp op)
{
	colorBlendAttachment.blendEnable = VK_TRUE;

	colorBlendAttachment.srcColorBlendFactor = src;
	colorBlendAttachment.dstColorBlendFactor = dst;
	colorBlendAttachment.colorBlendOp = op;

	return *this;
}

//透明度のブレンドの仕方を設定する
PipelineBuilder PipelineBuilder::withAlphaBlendFactorOp(const VkBlendFactor& src, const VkBlendFactor& dst, const VkBlendOp op)
{
	colorBlendAttachment.srcAlphaBlendFactor = src;
	colorBlendAttachment.dstAlphaBlendFactor = dst;
	colorBlendAttachment.alphaBlendOp = op;

	return *this;
}

//アタッチメントを追加
PipelineBuilder PipelineBuilder::addColoarAttachment()
{
	property.colorBlendStateArray.push_back(colorBlendAttachment);

	colorBlendAttachment = VkPipelineColorBlendAttachmentState{};

	return *this;
}
//ブレンド定数を設定
PipelineBuilder PipelineBuilder::withBlendConstant(const float& r, const float& g, const float& b, const float& a)
{
	property.colorBlendState.blendConstants[0] = r;
	property.colorBlendState.blendConstants[1] = g;
	property.colorBlendState.blendConstants[2] = b;
	property.colorBlendState.blendConstants[3] = a;

	return *this;
}

//動的に変更するステートを積み上げる
PipelineBuilder PipelineBuilder::addDynamicState(const VkDynamicState& state)
{
	property.dynamicStateArray.push_back(state);

	return *this;
}