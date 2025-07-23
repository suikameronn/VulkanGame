#include"FrameBufferFactory.h"

FrameBufferFactory::FrameBufferFactory(VkDevice d, std::shared_ptr<FrameBufferBuilder> b)
{
	builder = b;

	device = d;

	frameIndex = 1;
}

std::shared_ptr<FrameBuffer> FrameBufferFactory::Create(const FrameBufferProperty& property)
{
	std::shared_ptr<FrameBuffer> frameBuffer = std::make_shared<FrameBuffer>(shared_from_this());

	frameBuffer->renderPass = property.renderPass;

	frameBuffer->texture.resize(property.texture.size());
	std::copy(property.texture.begin(), property.texture.end(), frameBuffer->texture.begin());

	builder->Create(frameBuffer->frameBuffer, property);

	return frameBuffer;
}

void FrameBufferFactory::addDefferedDestruct(VkFramebuffer& frameBuffer)
{
	destructList[frameIndex].push_back(frameBuffer);
}

void FrameBufferFactory::resourceDestruct()
{
	for(auto& frameBuffer : destructList[frameIndex])
	{
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}

	frameIndex = (frameIndex == 0) ? 1 : 0; //フレームインデックスを切り替える
}