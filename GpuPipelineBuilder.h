#pragma once

#include"VulkanCore.h"
#include"ShaderFactory.h"
#include"GpuPipelineLayoutFactory.h"
#include"GpuRenderPassFactory.h"

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

class GpuPipelineBuilder
{
private:

	VkDevice device;

	//パイプラインの設定
	PipelineProperty property;

	std::shared_ptr<ShaderFactory> shaderFactory;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;

public:

	GpuPipelineBuilder(VkDevice& d, std::shared_ptr<ShaderFactory> sf);

	PipelineProperty Build();

	void Create(const PipelineProperty& property, VkPipeline& pipeline);

	//パイプラインの設定の初期化
	void initProperty();

	//レンダーパスを設定する
	GpuPipelineBuilder withRenderPass(const std::shared_ptr<RenderPass>& renderPass);
	//パイプラインレイアウトを設定する
	GpuPipelineBuilder withPipelineLayout(const std::shared_ptr<PipelineLayout>& pLayout);
	//レンダーパスを設定する
	GpuPipelineBuilder withRenderPass(const RenderPass& renderPass);

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
	GpuPipelineBuilder withCullMode(const VkCullModeFlags& mode);
	//ポリゴンの表裏判定を右回りか左回りに設定
	GpuPipelineBuilder withFrontFace(const VkFrontFace& face);
	//デプスバイアスを設定する
	GpuPipelineBuilder enableDepthBias(const bool& isBias);


	//マルチサンプリングシェーディングを設定
	GpuPipelineBuilder enableMultiSampleShading(const bool& mode);
	//最低のサンプリングシェーディングポイントの数を設定
	GpuPipelineBuilder withMinSampleShading(const float& min);
	//マルチサンプリングを行う際のサンプル数を設定
	GpuPipelineBuilder withRansterizationSamples(const VkSampleCountFlagBits& flag);


	//デプステストを設定する
	GpuPipelineBuilder enableDepthTest(const bool& isEnable);
	//zバッファへの書き込みを設定する
	GpuPipelineBuilder enableDepthWrite(const bool& isWrite);
	//z値の比較の仕方を設定する
	GpuPipelineBuilder withDepthCompare(const VkCompareOp& compare);
	//z値の範囲を指定し、それに満たない頂点は破棄する
	GpuPipelineBuilder enableDepthBoundsTest(const float& min, const float& max);
	//ステンシルテストを設定する
	GpuPipelineBuilder enableStencilTest(const VkStencilOpState& front, const VkStencilOpState& back);

	//アタッチメントの色の書き込みを設定
	GpuPipelineBuilder withColorWriteMask(const VkColorComponentFlags& flag);
	//色のブレンドの仕方を設定する
	GpuPipelineBuilder withColorBlendFactorOp(const VkBlendFactor& src,const VkBlendFactor& dst,const VkBlendOp op);
	//透明度のブレンドの仕方を設定する
	GpuPipelineBuilder withAlphaBlendFactorOp(const VkBlendFactor& src, const VkBlendFactor& dst, const VkBlendOp op);
	//アタッチメントを追加
	GpuPipelineBuilder addColoarAttachment();

	//ロジック演算を設定
	GpuPipelineBuilder withLogicOp(const VkLogicOp& logic);
	//ブレンド定数を設定
	GpuPipelineBuilder withBlendConstant(const float& r, const float& g, const float& b, const float& a);

	//動的に変更するステートを積み上げる
	GpuPipelineBuilder addDynamicState(const VkDynamicState& state);
};