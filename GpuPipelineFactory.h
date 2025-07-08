#pragma once

#include"GpuPipelineLayoutFactory.h"
#include"GpuPipelineBuilder.h"

struct Pipeline;

struct PipelineHash
{
	const uint64_t FNV_PRIME = 1099511628211LLU; // 2^40 + 2^8 + 0xB3 = 0x100000001b3
	const uint64_t FNV_OFFSET_BIAS = 14695981039346656037U; // 0xcbf29ce484222325

	void getHash(const uint32_t u, size_t& hash) const
	{
		uint64_t u64 = u;

		hash ^= u64;
		hash *= FNV_PRIME;
	}

	void getHash(const float f, size_t& hash) const
	{
		uint32_t u;
		std::memcpy(&u, &f, sizeof(float));

		getHash(u, hash);
	}

	void getHash(const VkPipelineShaderStageCreateInfo& info, size_t& hash) const
	{
		//すこしハッシュ値の漏れがある
		getHash(static_cast<uint32_t>(info.flags), hash);
		getHash(static_cast<uint32_t>(info.stage), hash);
	}

	void getHash(const VkPipelineVertexInputStateCreateInfo& info, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(info.vertexBindingDescriptionCount), hash);
		getHash(static_cast<uint32_t>(info.vertexAttributeDescriptionCount), hash);

		for (int i = 0; i < info.vertexBindingDescriptionCount; i++)
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription = info.pVertexBindingDescriptions[i];

			getHash(static_cast<uint32_t>(bindingDescription.binding), hash);
			getHash(static_cast<uint32_t>(bindingDescription.inputRate), hash);
			getHash(static_cast<uint32_t>(bindingDescription.stride), hash);
		}

		for (int i = 0; i < info.vertexAttributeDescriptionCount; i++)
		{
			VkVertexInputAttributeDescription attributeDescriptions{};
			attributeDescriptions = info.pVertexAttributeDescriptions[i];

			getHash(static_cast<uint32_t>(attributeDescriptions.binding), hash);
			getHash(static_cast<uint32_t>(attributeDescriptions.format), hash);
			getHash(static_cast<uint32_t>(attributeDescriptions.location), hash);
			getHash(static_cast<uint32_t>(attributeDescriptions.offset), hash);
		}
	}

	void getHash(const VkPipelineInputAssemblyStateCreateInfo& info, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(info.topology), hash);
		getHash(static_cast<uint32_t>(info.primitiveRestartEnable), hash);
	}

	void getHash(const VkPipelineTessellationStateCreateInfo& info, size_t& hash) const
	{
		//ハッシュ値の漏れがある
		getHash(static_cast<uint32_t>(info.flags), hash);
		getHash(static_cast<uint32_t>(info.patchControlPoints), hash);
	}

	void getHash(const VkPipelineViewportStateCreateInfo& info, size_t& hash) const
	{
		//ハッシュ値の漏れがある
		getHash(static_cast<uint32_t>(info.flags), hash);
		getHash(static_cast<uint32_t>(info.viewportCount), hash);
		getHash(static_cast<uint32_t>(info.scissorCount), hash);
	}

	void getHash(const VkPipelineRasterizationStateCreateInfo& info, size_t& hash) const
	{
		getHash(static_cast<float>(info.depthBiasClamp), hash);
		getHash(static_cast<uint32_t>(info.rasterizerDiscardEnable), hash);
		getHash(static_cast<uint32_t>(info.polygonMode), hash);
		getHash(static_cast<float>(info.lineWidth), hash);
		getHash(static_cast<uint32_t>(info.cullMode), hash);
		getHash(static_cast<uint32_t>(info.frontFace), hash);
		getHash(static_cast<uint32_t>(info.depthBiasEnable), hash);
	}

	void getHash(const VkPipelineMultisampleStateCreateInfo& info, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(info.sampleShadingEnable), hash);
		getHash(static_cast<float>(info.minSampleShading), hash);
		getHash(static_cast<uint32_t>(info.rasterizationSamples), hash);
	}

	void getHash(const VkPipelineDepthStencilStateCreateInfo& info, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(info.depthTestEnable), hash);
		getHash(static_cast<uint32_t>(info.depthWriteEnable), hash);
		getHash(static_cast<uint32_t>(info.depthCompareOp), hash);
		getHash(static_cast<uint32_t>(info.depthBoundsTestEnable), hash);
		getHash(static_cast<float>(info.minDepthBounds), hash);
		getHash(static_cast<float>(info.maxDepthBounds), hash);
		getHash(static_cast<uint32_t>(info.stencilTestEnable), hash);

		getHash(static_cast<uint32_t>(info.front.failOp), hash);
		getHash(static_cast<uint32_t>(info.front.passOp), hash);
		getHash(static_cast<uint32_t>(info.front.depthFailOp), hash);
		getHash(static_cast<uint32_t>(info.front.compareOp), hash);
		getHash(info.front.compareMask, hash);
		getHash(info.front.writeMask, hash);
		getHash(info.front.reference, hash);

		getHash(static_cast<uint32_t>(info.back.failOp), hash);
		getHash(static_cast<uint32_t>(info.back.passOp), hash);
		getHash(static_cast<uint32_t>(info.back.depthFailOp), hash);
		getHash(static_cast<uint32_t>(info.back.compareOp), hash);
		getHash(info.back.compareMask, hash);
		getHash(info.back.writeMask, hash);
		getHash(info.back.reference, hash);
	}

	void getHash(const VkPipelineColorBlendAttachmentState& info, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(info.colorWriteMask), hash);
		getHash(static_cast<uint32_t>(info.blendEnable), hash);
		getHash(static_cast<uint32_t>(info.srcColorBlendFactor), hash);
		getHash(static_cast<uint32_t>(info.dstColorBlendFactor), hash);
		getHash(static_cast<uint32_t>(info.colorBlendOp), hash);
		getHash(static_cast<uint32_t>(info.srcAlphaBlendFactor), hash);
		getHash(static_cast<uint32_t>(info.dstAlphaBlendFactor), hash);
		getHash(static_cast<uint32_t>(info.alphaBlendOp), hash);
	}

	void getHash(const VkPipelineColorBlendStateCreateInfo& info, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(info.logicOp), hash);
		getHash(static_cast<uint32_t>(info.attachmentCount), hash);

		for (int i = 0; i < info.attachmentCount; i++)
		{
			getHash(info.pAttachments[i], hash);
		}

		for (int i = 0; i < 4; i++)
		{
			getHash(static_cast<float>(info.blendConstants[i]), hash);
		}
	}

	void getHash(const VkPipelineDynamicStateCreateInfo& info, size_t& hash) const
	{
		getHash(info.dynamicStateCount, hash);

		for (int i = 0; i < info.dynamicStateCount; i++)
		{
			getHash(static_cast<uint32_t>(info.pDynamicStates[i]), hash);
		}
	}

	size_t operator()(const PipelineProperty& a) const
	{
		size_t hash = FNV_OFFSET_BIAS;

		getHash(static_cast<uint32_t>(a.isComputePipeline), hash);
		getHash(static_cast<uint32_t>(a.stageCount), hash);
		
		std::hash<std::string> strHasher;
		for (const auto str : a.shaderPaths)
		{
			hash ^= strHasher(str);
		}

		for (const VkPipelineShaderStageCreateInfo& info : a.stages)
		{
			getHash(info, hash);
		}

		getHash(a.vertexInputState, hash);

		getHash(a.inputAssemblyState, hash);

		getHash(a.tessellationState, hash);

		getHash(a.viewportState, hash);

		getHash(a.rasterizationState, hash);

		getHash(a.multisampleState, hash);

		getHash(a.depthStencilState, hash);

		getHash(a.colorBlendState, hash);

		getHash(a.dynamicState, hash);
	}
};

class GpuPipelineFactory
{
private:
	//論理デバイス
	VkDevice device;

	//現在の破棄予定のリストのインデックス
	//描画処理が行われるごとに値が更新される
	uint32_t frameIndex;

	PipelineLayoutHash pipelineLayoutHash;

	//ビルダー
	std::shared_ptr<GpuPipelineBuilder> builder;

	//レイアウトを取得する
	std::shared_ptr<GpuPipelineLayoutFactory> pLayoutFactory;

	//既に作成したパイプラインレイアウトを格納する
	std::unordered_map<std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>, std::vector<VkPushConstantRange>>
		, std::weak_ptr<Pipeline>, PipelineLayoutHash> pipelineLayoutStorage;

	//破棄予定のリソースのリスト
	std::array<std::list<VkPipeline>, 2> destructList;

public:

	GpuPipelineFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutFactory> f);

	~GpuPipelineFactory();

	//遅延破棄リストにリソースを追加する
	void addDefferedDestruct(VkPipeline& pLayout);

	//リソースを破棄する
	void resourceDestruct();
};

struct Pipeline
{
	size_t hashKey;

	VkPipeline pipeline;

	std::shared_ptr<GpuPipelineFactory> factory;

	Pipeline(std::shared_ptr<GpuPipelineFactory> f)
	{
		hashKey = 0;
		pipeline = nullptr;

		factory = f;
	}

	~Pipeline()
	{
		factory->addDefferedDestruct(pipeline);
	}
};