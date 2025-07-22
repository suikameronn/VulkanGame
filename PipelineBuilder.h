#pragma once

#include"VulkanCore.h"
#include"ShaderFactory.h"
#include"PipelineLayoutFactory.h"
#include"RenderPassFactory.h"

struct PipelineProperty
{
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
};

class PipelineBuilder
{
private:

	VkDevice device;

	//�p�C�v���C���̐ݒ�
	PipelineProperty property;

	std::shared_ptr<ShaderFactory> shaderFactory;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;

public:

	PipelineBuilder(VkDevice& d, std::shared_ptr<ShaderFactory> sf);

	PipelineProperty Build();

	void Create(const PipelineProperty& property, VkPipeline& pipeline);

	//�p�C�v���C���̐ݒ�̏�����
	void initProperty();

	//�����_�[�p�X��ݒ肷��
	PipelineBuilder withRenderPass(const std::shared_ptr<RenderPass>& renderPass);
	//�p�C�v���C�����C�A�E�g��ݒ肷��
	PipelineBuilder withPipelineLayout(const std::shared_ptr<PipelineLayout>& pLayout);
	//�����_�[�p�X��ݒ肷��
	PipelineBuilder withRenderPass(const RenderPass& renderPass);

	//���_�V�F�[�_�p�X��ݒ�
	PipelineBuilder withVertexShader(const std::string& path);
	//�t���O�����g�V�F�[�_�p�X��ݒ�
	PipelineBuilder withFragmentShader(const std::string& path);
	//�R���s���[�g�V�F�[�_
	PipelineBuilder withComputeShader(const std::string& path);

	//���_�f�[�^�̓ǂݎ�����ݒ�
	PipelineBuilder withVertexInput(const uint32_t& binding, const VkVertexInputRate& rate
		, const uint32_t& stride);

	//���_�A�g���r���[�g�̒ǉ�
	PipelineBuilder addVertexInputAttrib(const uint32_t& location, const uint32_t& binding,
		const VkFormat& format, const uint32_t& offset);

	//�v���~�e�B�u�̐ݒ�
	PipelineBuilder withTopology(const VkPrimitiveTopology& topology);


	//�v���~�e�B�u�̕`����@��ݒ�
	PipelineBuilder withPolygonMode(const VkPolygonMode& mode);
	//���C���̑�����ݒ�
	PipelineBuilder withLineWidth(const float& width);
	//�J�����O���[�h�̐ݒ�
	PipelineBuilder withCullMode(const VkCullModeFlags& mode);
	//�|���S���̕\��������E��肩�����ɐݒ�
	PipelineBuilder withFrontFace(const VkFrontFace& face);
	//�f�v�X�o�C�A�X��ݒ肷��
	PipelineBuilder enableDepthBias(const bool& isBias);


	//�}���`�T���v�����O�V�F�[�f�B���O��ݒ�
	PipelineBuilder enableMultiSampleShading(const bool& mode);
	//�Œ�̃T���v�����O�V�F�[�f�B���O�|�C���g�̐���ݒ�
	PipelineBuilder withMinSampleShading(const float& min);
	//�}���`�T���v�����O���s���ۂ̃T���v������ݒ�
	PipelineBuilder withRansterizationSamples(const VkSampleCountFlagBits& flag);


	//�f�v�X�e�X�g��ݒ肷��
	PipelineBuilder enableDepthTest(const bool& isEnable);
	//z�o�b�t�@�ւ̏������݂�ݒ肷��
	PipelineBuilder enableDepthWrite(const bool& isWrite);
	//z�l�̔�r�̎d����ݒ肷��
	PipelineBuilder withDepthCompare(const VkCompareOp& compare);
	//z�l�͈̔͂��w�肵�A����ɖ����Ȃ����_�͔j������
	PipelineBuilder enableDepthBoundsTest(const float& min, const float& max);
	//�X�e���V���e�X�g��ݒ肷��
	PipelineBuilder enableStencilTest(const VkStencilOpState& front, const VkStencilOpState& back);

	//�A�^�b�`�����g�̐F�̏������݂�ݒ�
	PipelineBuilder withColorWriteMask(const VkColorComponentFlags& flag);
	//�F�̃u�����h�̎d����ݒ肷��
	PipelineBuilder withColorBlendFactorOp(const VkBlendFactor& src,const VkBlendFactor& dst,const VkBlendOp op);
	//�����x�̃u�����h�̎d����ݒ肷��
	PipelineBuilder withAlphaBlendFactorOp(const VkBlendFactor& src, const VkBlendFactor& dst, const VkBlendOp op);
	//�A�^�b�`�����g��ǉ�
	PipelineBuilder addColoarAttachment();

	//���W�b�N���Z��ݒ�
	PipelineBuilder withLogicOp(const VkLogicOp& logic);
	//�u�����h�萔��ݒ�
	PipelineBuilder withBlendConstant(const float& r, const float& g, const float& b, const float& a);

	//���I�ɕύX����X�e�[�g��ςݏグ��
	PipelineBuilder addDynamicState(const VkDynamicState& state);
};