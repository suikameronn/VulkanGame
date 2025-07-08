#pragma once

#include"VulkanCore.h"

struct PipelineProperty
{
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	std::string computeShaderpath;

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
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	VkPipelineDynamicStateCreateInfo dynamicState;

	void initProperty()
	{
		vertexShaderPath = "";
		fragmentShaderPath = "";
		computeShaderpath = "";
		stages.clear();
		bindingDescription = VkVertexInputBindingDescription{};
		attributeDescriptions.clear();
		vertexInputState = VkPipelineVertexInputStateCreateInfo{};
		inputAssemblyState = VkPipelineInputAssemblyStateCreateInfo{};
		tessellationState = VkPipelineTessellationStateCreateInfo{};
		viewportState = VkPipelineViewportStateCreateInfo{};
		rasterizationState = VkPipelineRasterizationStateCreateInfo{};
		multisampleState = VkPipelineMultisampleStateCreateInfo{};
		depthStencilState = VkPipelineDepthStencilStateCreateInfo{};
		colorBlendState = VkPipelineColorBlendStateCreateInfo{};
		dynamicState = VkPipelineDynamicStateCreateInfo{};
	}
};

class GpuPipelineBuilder
{
private:

	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	//�p�C�v���C���̐ݒ�
	PipelineProperty property{};

public:

	GpuPipelineBuilder(VkDevice& d, std::shared_ptr<VulkanCore> core);

	//�p�C�v���C���̐ݒ�̏�����
	void initProperty();

	//���_�V�F�[�_�p�X��ݒ�
	GpuPipelineBuilder withVertexShader(const std::string& path);
	//�t���O�����g�V�F�[�_�p�X��ݒ�
	GpuPipelineBuilder withFragmentShader(const std::string& path);
	//�R���s���[�g�V�F�[�_
	GpuPipelineBuilder withComputeShader(const std::string& path);

	//���_�f�[�^�̓ǂݎ�����ݒ�
	GpuPipelineBuilder withVertexInput(const uint32_t& binding, const VkVertexInputRate& rate
		, const uint32_t& stride);

	//���_�A�g���r���[�g�̒ǉ�
	GpuPipelineBuilder addVertexInputAttrib(const uint32_t& location, const uint32_t& binding,
		const VkFormat& format, const uint32_t& offset);

	//�v���~�e�B�u�̐ݒ�
	GpuPipelineBuilder withTopology(const VkPrimitiveTopology& topology);


	//�v���~�e�B�u�̕`����@��ݒ�
	GpuPipelineBuilder withPolygonMode(const VkPolygonMode& mode);
	//���C���̑�����ݒ�
	GpuPipelineBuilder withLineWidth(const float& width);
	//�J�����O���[�h�̐ݒ�
	GpuPipelineBuilder withCulModel(const VkCullModeFlags& mode);
	//�|���S���̕\��������E��肩�����ɐݒ�
	GpuPipelineBuilder withFrontFace(const VkFrontFace& face);

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.minSampleShading = 0.2f;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//�}���`�T���v�����O��L���ɂ���
	GpuPipeineBuilder with
};