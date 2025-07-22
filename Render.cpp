#include"Render.h"

Render::Render(std::shared_ptr<VulkanCore> core, std::shared_ptr<GpuBufferFactory> bf
	, std::shared_ptr<GltfModelFactory> model, std::shared_ptr<ECSManager> ecs)
{
	vulkanCore = core;

	modelFactory = model;

	ecsManager = ecs;

	device = core->getLogicDevice();

	initProperty();
}

Render Render::initProperty()
{
	property.initProperty();

	return *this;
}

//レンダーパスを設定
Render Render::withRenderPass(const std::shared_ptr<RenderPass> pass)
{
	property.renderPass = pass;
	property.info.renderPass = pass->renderPass;

	return *this;
}

//フレームバッファの設定
Render Render::withFrameBuffer(const std::shared_ptr<FrameBuffer> frameBuffer)
{
	property.frameBuffer = frameBuffer;
	property.info.framebuffer = frameBuffer->frameBuffer;

	return *this;
}

//レンダー範囲の設定
Render Render::withRenderArea(const uint32_t& width, const uint32_t& height)
{
	property.info.renderArea.extent = { width,height };

	return *this;
}

//レンダー画像の初期化色の設定
Render Render::withClearColor(const glm::ivec4& color)
{
	property.isClear = true;

	for (int i = 0; i < 4; i++)
	{
		property.clearValues[0].color.int32[i] = color[i];
	}

	return *this;
}

//レンダー画像の初期化のデプスの設定
Render Render::withClearDepth(const float& depth)
{
	property.isClear = true;

	property.clearValues[1].depthStencil.depth = depth;

	return *this;
}

//レンダー画像の初期化のステンシルの設定
Render Render::withClearStencil(const uint32_t& stencil)
{
	property.isClear = true;

	property.clearValues[1].depthStencil.stencil = stencil;

	return *this;
}

//コマンドを積むコマンドバッファを設定
Render Render::withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer)
{
	property.commandBuffer = commandBuffer;

	return *this;
}

//プロパティを返す
RenderProperty Render::Build()
{
	if (property.isClear)
	{
		property.info.clearValueCount = static_cast<uint32_t>(property.clearValues.size());
		property.info.pClearValues = property.clearValues.data();
	}

	RenderProperty p = property;

	property.initProperty();

	return p;
}

//レンダリング開始
void Render::RenderStart(const RenderProperty& property)
{
	vkCmdBeginRenderPass(property.commandBuffer->commandBuffer, &property.info
		, VK_SUBPASS_CONTENTS_INLINE);
}

void Render::Rendering()  
{
	ecsManager->RunFunction<GltfModelComp>
		(
			{ [](GltfModelComp& Comp)
			{

			}
			}
		);
}


//レンダリング終了
void Render::RenderEnd()
{
	vkCmdEndRenderPass(property.commandBuffer->commandBuffer);
}