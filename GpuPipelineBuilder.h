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

	//パイプラインの設定
	PipelineProperty property{};

public:

	GpuPipelineBuilder(VkDevice& d, std::shared_ptr<VulkanCore> core);

	//パイプラインの設定の初期化
	void initProperty();

	//頂点シェーダパスを設定
	GpuPipelineBuilder withVertexShader(const std::string& path);
	//フラグメントシェーダパスを設定
	GpuPipelineBuilder withFragmentShader(const std::string& path);
	//コンピュートシェーダ
	GpuPipelineBuilder withComputeShader(const std::string& path);

	//頂点データの読み取り方を設定
	GpuPipelineBuilder withVertexInput(const uint32_t& binding, const VkVertexInputRate& rate
		, const uint32_t& stride);

	//頂点アトリビュートの追加
	GpuPipelineBuilder addVertexInputAttrib(const uint32_t& location, const uint32_t& binding,
		const VkFormat& format, const uint32_t& offset);

	//プリミティブの設定
	GpuPipelineBuilder withTopology(const VkPrimitiveTopology& topology);


	//プリミティブの描画方法を設定
	GpuPipelineBuilder withPolygonMode(const VkPolygonMode& mode);
	//ラインの太さを設定
	GpuPipelineBuilder withLineWidth(const float& width);
	//カリングモードの設定
	GpuPipelineBuilder withCulModel(const VkCullModeFlags& mode);
	//ポリゴンの表裏判定を右回りか左回りに設定
	GpuPipelineBuilder withFrontFace(const VkFrontFace& face);

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.minSampleShading = 0.2f;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//マルチサンプリングを有効にする
	GpuPipeineBuilder with
};