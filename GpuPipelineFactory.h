#pragma once

#include"GpuPipelineBuilder.h"
#include"GpuRenderPassFactory.h"

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
		//�������n�b�V���l�̘R�ꂪ����
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
		//�n�b�V���l�̘R�ꂪ����
		getHash(static_cast<uint32_t>(info.flags), hash);
		getHash(static_cast<uint32_t>(info.patchControlPoints), hash);
	}

	void getHash(const VkPipelineViewportStateCreateInfo& info, size_t& hash) const
	{
		//�n�b�V���l�̘R�ꂪ����
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

	void getHash(const std::shared_ptr<PipelineLayout>& pLayout, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(pLayout->hashKey), hash);
	}

	void getHash(const std::shared_ptr<RenderPass>& renderPass, size_t& hash) const
	{
		getHash(static_cast<uint32_t>(renderPass->hashKey), hash);
	}

	size_t operator()(const PipelineProperty& a) const
	{
		size_t hash = FNV_OFFSET_BIAS;
		
		std::hash<std::string> strHasher;
		hash ^= strHasher(a.vertexShader->path);
		hash ^= strHasher(a.fragmentShader->path);
		hash ^= strHasher(a.computeShader->path);

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

		getHash(a.pLayout, hash);

		getHash(a.renderPass, hash);
	}
};

enum PipelinePattern
{
	PBR,
	UI,
	CUBEMAP,
	CALC_SHADOWMAP,
	CALC_CUBEMAP,
	CALC_IBL_DIFFUSE,
	CALC_IBL_SPECULAR,
	CALC_IBL_BRDF,
	RAYCAST
};

class GpuPipelineFactory : public std::enable_shared_from_this<GpuPipelineFactory>
{
private:
	//�_���f�o�C�X
	VkDevice device;

	//���݂̔j���\��̃��X�g�̃C���f�b�N�X
	//�`�揈�����s���邲�Ƃɒl���X�V�����
	uint32_t frameIndex;

	PipelineHash pipelineHash;

	//�r���_�[
	std::shared_ptr<GpuPipelineBuilder> builder;

	//���C�A�E�g���擾����
	std::shared_ptr<GpuPipelineLayoutFactory> pLayoutFactory;

	//�V�F�[�_���W���[�������o��
	std::shared_ptr<Shader> shaderFactory;
	
	//�����_�[�p�X�����o��
	std::shared_ptr<GpuRenderPassFactory> renderPassFactory;

	//���ɍ쐬�����p�C�v���C�����C�A�E�g���i�[����
	std::unordered_map<Pipeline, std::weak_ptr<Pipeline>, PipelineHash> pipelineStorage;

	//�j���\��̃��\�[�X�̃��X�g
	std::array<std::list<VkPipeline>, 2> destructList;

	PipelineProperty convertPattern(const PipelinePattern& pattern);

public:

	GpuPipelineFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutFactory> f
		, std::shared_ptr<Shader> sf, std::shared_ptr<GpuPipelineBuilder> b
		,std::shared_ptr<GpuRenderPassFactory> r);

	~GpuPipelineFactory();

	//�p�C�v���C�����쐬����
	std::shared_ptr<Pipeline> Create(const PipelinePattern& pattern);

	//�p�C�v���C�����쐬����
	std::shared_ptr<Pipeline> Create(const PipelineProperty& property);

	//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
	void addDefferedDestruct(VkPipeline& pLayout);

	//���\�[�X��j������
	void resourceDestruct();
};

struct Pipeline
{
	size_t hashKey;

	VkPipeline pipeline;

	std::shared_ptr<PipelineLayout> pLayout;

	std::shared_ptr<Shader> shader;
	std::shared_ptr<GpuPipelineFactory> factory;

	Pipeline(std::shared_ptr<GpuPipelineFactory> f)
	{
		hashKey = 0;
		pipeline = nullptr;
		pLayout = nullptr;

		factory = f;
	}

	~Pipeline()
	{
		factory->addDefferedDestruct(pipeline);
	}
};