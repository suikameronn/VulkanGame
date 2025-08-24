#include"Render.h"

Render::Render(VkDevice& d)
{
	device = d;
}

std::shared_ptr<Render> Render::initProperty()
{
	property.initProperty();

	return shared_from_this();
}

//�����_�[�p�X��ݒ�
std::shared_ptr<Render> Render::withRenderPass(const std::shared_ptr<RenderPass> pass)
{
	property.renderPass = pass;
	property.info.renderPass = pass->renderPass;

	return shared_from_this();
}

//�t���[���o�b�t�@�̐ݒ�
std::shared_ptr<Render> Render::withFrameBuffer(const std::shared_ptr<FrameBuffer> frameBuffer)
{
	property.frameBuffer = frameBuffer;
	property.info.framebuffer = frameBuffer->frameBuffer;

	return shared_from_this();
}

//�����_�[�͈͂̐ݒ�
std::shared_ptr<Render> Render::withRenderArea(const uint32_t& width, const uint32_t& height)
{
	property.info.renderArea.extent = { width,height };

	return shared_from_this();
}

//�����_�[�摜�̏������F�̐ݒ�
std::shared_ptr<Render> Render::withClearColor(const glm::ivec4& color)
{
	property.isColorClear = true;

	for (int i = 0; i < 4; i++)
	{
		property.colorClear.color.int32[i] = color[i];
	}

	return shared_from_this();
}

//�����_�[�摜�̏������̃f�v�X�̐ݒ�
std::shared_ptr<Render> Render::withClearDepth(const float& depth)
{
	property.isDepthStencilClear = true;

	property.depthStencilClear.depthStencil.depth = depth;

	return shared_from_this();
}

//�����_�[�摜�̏������̃X�e���V���̐ݒ�
std::shared_ptr<Render> Render::withClearStencil(const uint32_t& stencil)
{
	property.isDepthStencilClear = true;

	property.depthStencilClear.depthStencil.stencil = stencil;

	return shared_from_this();
}

//�R�}���h��ςރR�}���h�o�b�t�@��ݒ�
std::shared_ptr<Render> Render::withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer)
{
	property.commandIndex = 0;
	property.commandBuffer = commandBuffer;

	return shared_from_this();
}

//�R�}���h��ςރR�}���h�o�b�t�@��ݒ�
std::shared_ptr<Render> Render::withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer, const uint32_t& index)
{
	property.commandIndex = index;
	property.commandBuffer = commandBuffer;

	return shared_from_this();
}

//�v���p�e�B��Ԃ�
RenderProperty Render::Build()
{
	if(property.isColorClear && property.isDepthStencilClear)
	{
		property.info.clearValueCount = 2;
		property.clearValues.resize(2);
		property.clearValues[0] = property.colorClear;
		property.clearValues[1] = property.depthStencilClear;
		property.info.pClearValues = property.clearValues.data();
	}
	else if(property.isColorClear)
	{
		property.info.clearValueCount = 1;
		property.clearValues.resize(1);
		property.clearValues[0] = property.colorClear;
		property.info.pClearValues = property.clearValues.data();
	}
	else if(property.isDepthStencilClear)
	{
		property.info.clearValueCount = 1;
		property.clearValues.resize(1);
		property.clearValues[0] = property.depthStencilClear;
		property.info.pClearValues = property.clearValues.data();
	}
	else
	{
		property.info.clearValueCount = 0;
		property.info.pClearValues = nullptr;
	}

	return property;
}

//�����_�����O�J�n
void Render::RenderStart(const RenderProperty& property)
{
	vkCmdBeginRenderPass(property.commandBuffer->getCommand(property.commandIndex), &property.info
		, VK_SUBPASS_CONTENTS_INLINE);
}

//�����_�����O�I��
void Render::RenderEnd(const RenderProperty& property)
{
	vkCmdEndRenderPass(property.commandBuffer->getCommand(property.commandIndex));

	initProperty();
}