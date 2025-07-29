#pragma once

#include<memory>
#include<array>

#include"RenderPassFactory.h"
#include"DescriptorSetLayoutFactory.h"
#include"FrameBufferFactory.h"
#include"SwapChain.h"
#include"GpuBufferFactory.h"
#include"VulkanCore.h"

#include<glm/glm.hpp>

struct RenderProperty
{
	VkRenderPassBeginInfo info;

	std::shared_ptr<RenderPass> renderPass;

	std::shared_ptr<FrameBuffer> frameBuffer;

	std::shared_ptr<CommandBuffer> commandBuffer;

	bool isClear;
	std::array<VkClearValue, 2> clearValues;

	void initProperty()
	{
		info = VkRenderPassBeginInfo{};

		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		info.renderArea.offset = { 0,0 };

		renderPass.reset();

		frameBuffer.reset();

		isClear = false;

		clearValues[0].color = { {0.0f,0.0f,0.0f,1.0f} };
		clearValues[1].depthStencil = { 1.0f,0 };
	};
};

class Render
{
private:

	VkDevice device;

	RenderProperty property;

public:

	Render(VkDevice& d);

	~Render() = default;

	Render initProperty();

	//レンダーパスを設定
	Render withRenderPass(const std::shared_ptr<RenderPass> pass);

	//フレームバッファの設定
	Render withFrameBuffer(const std::shared_ptr<FrameBuffer> frameBuffer);

	//レンダー範囲の設定
	Render withRenderArea(const uint32_t& width, const uint32_t& height);

	//レンダー画像の初期化色の設定
	Render withClearColor(const glm::ivec4& color);

	//レンダー画像の初期化のデプスの設定
	Render withClearDepth(const float& depth);

	//レンダー画像の初期化のステンシルの設定
	Render withClearStencil(const uint32_t& stencil);

	//コマンドを積むコマンドバッファを設定
	Render withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer);

	//プロパティを返す
	RenderProperty Build();

	//レンダリング開始
	void RenderStart(const RenderProperty& property);

	//実際のレンダリングのコマンドの積み込みは
	//ecsManagerのRunFunctionを使う

	//レンダリング終了
	void RenderEnd(const RenderProperty& property);
};