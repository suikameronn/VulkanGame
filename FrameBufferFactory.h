#pragma once

#include"FrameBufferBuilder.h"

struct FrameBuffer;

class FrameBufferFactory : public std::enable_shared_from_this<FrameBufferFactory>
{
private:

	uint32_t frameIndex;

	VkDevice device;

	std::shared_ptr<FrameBufferBuilder> builder;

	std::array<std::list<VkFramebuffer>, 2> destructList;

public:

	FrameBufferFactory(VkDevice d, std::shared_ptr<FrameBufferBuilder> b);

	~FrameBufferFactory()
	{
		for (int i = 0; i < 2; i++)
		{
			resourceDestruct();
		}

#ifdef _DEBUG
		std::cout << "FrameBufferFactory :: デストラクタ" << std::endl;
#endif
	}

	//フレームバッファビルダーを取得
	std::shared_ptr<FrameBufferBuilder> getBuilder()
	{
		return builder;
	}

	std::shared_ptr<FrameBuffer> Create(const FrameBufferProperty& property);

	void addDefferedDestruct(VkFramebuffer& frameBuffer);

	void resourceDestruct();
};

struct FrameBuffer
{
	VkFramebuffer frameBuffer;

	std::shared_ptr<RenderPass> renderPass;
	std::vector<std::shared_ptr<Texture>> texture;

	std::shared_ptr<FrameBufferFactory> factory;

	FrameBuffer(std::shared_ptr<FrameBufferFactory> f)
	{
		frameBuffer = nullptr;

		factory = f;
	}

	~FrameBuffer()
	{
		factory->addDefferedDestruct(frameBuffer);
	}
};