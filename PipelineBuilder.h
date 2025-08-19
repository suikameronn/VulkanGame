#pragma once

#include"VulkanCore.h"
#include"ShaderFactory.h"
#include"PipelineLayoutFactory.h"
#include"RenderPassFactory.h"

struct PipelineProperty
{
	size_t hashKey;

	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> fragmentShader;
	std::shared_ptr<Shader> computeShader;

	std::vector<VkPipelineShaderStageCreateInfo> stages;
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	VkPipelineTessellationStateCreateInfo tessellationState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizationState;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendStateArray;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	std::vector<VkDynamicState> dynamicStateArray;
	VkPipelineDynamicStateCreateInfo dynamicState;

	std::shared_ptr<PipelineLayout> pLayout;
	std::shared_ptr<RenderPass> renderPass;

	void initProperty()
	{
		hashKey = 0;

		vertexShader = nullptr;
		fragmentShader = nullptr;
		computeShader = nullptr;

		pLayout = nullptr;

		renderPass = nullptr;
		
		stages.clear();
		
		bindingDescription = VkVertexInputBindingDescription{};
		
		attributeDescriptions.clear();

		vertexInputState = VkPipelineVertexInputStateCreateInfo{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		
		inputAssemblyState = VkPipelineInputAssemblyStateCreateInfo{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		tessellationState = VkPipelineTessellationStateCreateInfo{};
		tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

		viewportState = VkPipelineViewportStateCreateInfo{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		rasterizationState = VkPipelineRasterizationStateCreateInfo{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;

		multisampleState = VkPipelineMultisampleStateCreateInfo{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

		depthStencilState = VkPipelineDepthStencilStateCreateInfo{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		colorBlendStateArray.clear();
		colorBlendState = VkPipelineColorBlendStateCreateInfo{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		dynamicStateArray.clear();
		dynamicState = VkPipelineDynamicStateCreateInfo{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	}

	bool operator==(const PipelineProperty& other) const
	{
		return hashKey == other.hashKey;
	}
};

class PipelineBuilder : public std::enable_shared_from_this<PipelineBuilder>
{
private:

	VkDevice device;

	//�p�C�v���C���̐ݒ�
	PipelineProperty property;

	std::shared_ptr<ShaderFactory> shaderFactory;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;

public:

	PipelineBuilder(VkDevice& d, std::shared_ptr<ShaderFactory> sf);

	~PipelineBuilder()
	{
#ifdef _DEBUG
		std::cout << "PipelineBuilder :: �f�X�g���N�^" << std::endl;
#endif
	}

	PipelineProperty Build();

	void Create(const PipelineProperty& property, VkPipeline& pipeline);

	//�p�C�v���C���̐ݒ�̏�����
	std::shared_ptr<PipelineBuilder> initProperty();

	//�����_�[�p�X��ݒ肷��
	std::shared_ptr<PipelineBuilder> withRenderPass(const std::shared_ptr<RenderPass>& renderPass);
	//�p�C�v���C�����C�A�E�g��ݒ肷��
	std::shared_ptr<PipelineBuilder> withPipelineLayout(const std::shared_ptr<PipelineLayout>& pLayout);
	//�����_�[�p�X��ݒ肷��
	std::shared_ptr<PipelineBuilder> withRenderPass(const RenderPass& renderPass);

	//���_�V�F�[�_�p�X��ݒ�
	std::shared_ptr<PipelineBuilder> withVertexShader(const std::string& path);
	//�t���O�����g�V�F�[�_�p�X��ݒ�
	std::shared_ptr<PipelineBuilder> withFragmentShader(const std::string& path);
	//�R���s���[�g�V�F�[�_
	std::shared_ptr<PipelineBuilder> withComputeShader(const std::string& path);

	//���_�f�[�^�̓ǂݎ�����ݒ�
	std::shared_ptr<PipelineBuilder> withVertexInput(const uint32_t& binding, const VkVertexInputRate& rate
		, const uint32_t& stride);

	//���_�A�g���r���[�g�̒ǉ�
	std::shared_ptr<PipelineBuilder> addVertexInputAttrib(const uint32_t& location, const uint32_t& binding,
		const VkFormat& format, const uint32_t& offset);

	//�v���~�e�B�u�̐ݒ�
	std::shared_ptr<PipelineBuilder> withTopology(const VkPrimitiveTopology& topology);


	//�v���~�e�B�u�̕`����@��ݒ�
	std::shared_ptr<PipelineBuilder> withPolygonMode(const VkPolygonMode& mode);
	//���C���̑�����ݒ�
	std::shared_ptr<PipelineBuilder> withLineWidth(const float& width);
	//�J�����O���[�h�̐ݒ�
	std::shared_ptr<PipelineBuilder> withCullMode(const VkCullModeFlags& mode);
	//�|���S���̕\��������E��肩�����ɐݒ�
	std::shared_ptr<PipelineBuilder> withFrontFace(const VkFrontFace& face);
	//�f�v�X�o�C�A�X��ݒ肷��
	std::shared_ptr<PipelineBuilder> enableDepthBias(const bool& isBias);


	//�}���`�T���v�����O�V�F�[�f�B���O��ݒ�
	std::shared_ptr<PipelineBuilder> enableMultiSampleShading(const bool& mode);
	//�Œ�̃T���v�����O�V�F�[�f�B���O�|�C���g�̐���ݒ�
	std::shared_ptr<PipelineBuilder> withMinSampleShading(const float& min);
	//�}���`�T���v�����O���s���ۂ̃T���v������ݒ�
	std::shared_ptr<PipelineBuilder> withRansterizationSamples(const VkSampleCountFlagBits& flag);


	//�f�v�X�e�X�g��ݒ肷��
	std::shared_ptr<PipelineBuilder> enableDepthTest(const bool& isEnable);
	//z�o�b�t�@�ւ̏������݂�ݒ肷��
	std::shared_ptr<PipelineBuilder> enableDepthWrite(const bool& isWrite);
	//z�l�̔�r�̎d����ݒ肷��
	std::shared_ptr<PipelineBuilder> withDepthCompare(const VkCompareOp& compare);
	//z�l�͈̔͂��w�肵�A����ɖ����Ȃ����_�͔j������
	std::shared_ptr<PipelineBuilder> enableDepthBoundsTest(const float& min, const float& max);
	//�X�e���V���e�X�g��ݒ肷��
	std::shared_ptr<PipelineBuilder> enableStencilTest(const VkStencilOpState& front, const VkStencilOpState& back);

	//�A�^�b�`�����g�̐F�̏������݂�ݒ�
	std::shared_ptr<PipelineBuilder> withColorWriteMask(const VkColorComponentFlags& flag);
	//�F�̃u�����h�̎d����ݒ肷��
	std::shared_ptr<PipelineBuilder> withColorBlendFactorOp(const VkBlendFactor& src,const VkBlendFactor& dst,const VkBlendOp op);
	//�����x�̃u�����h�̎d����ݒ肷��
	std::shared_ptr<PipelineBuilder> withAlphaBlendFactorOp(const VkBlendFactor& src, const VkBlendFactor& dst, const VkBlendOp op);
	//�A�^�b�`�����g��ǉ�
	std::shared_ptr<PipelineBuilder> addColoarAttachment();

	//���W�b�N���Z��ݒ�
	std::shared_ptr<PipelineBuilder> withLogicOp(const VkLogicOp& logic);
	//�u�����h�萔��ݒ�
	std::shared_ptr<PipelineBuilder> withBlendConstant(const float& r, const float& g, const float& b, const float& a);

	//���I�ɕύX����X�e�[�g��ςݏグ��
	std::shared_ptr<PipelineBuilder> addDynamicState(const VkDynamicState& state);
};