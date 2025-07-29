#include"Render.h"

Render::Render(VkDevice& d)
{
	device = d;

	initProperty();
}

Render Render::initProperty()
{
	property.initProperty();

	return *this;
}

//�����_�[�p�X��ݒ�
Render Render::withRenderPass(const std::shared_ptr<RenderPass> pass)
{
	property.renderPass = pass;
	property.info.renderPass = pass->renderPass;

	return *this;
}

//�t���[���o�b�t�@�̐ݒ�
Render Render::withFrameBuffer(const std::shared_ptr<FrameBuffer> frameBuffer)
{
	property.frameBuffer = frameBuffer;
	property.info.framebuffer = frameBuffer->frameBuffer;

	return *this;
}

//�����_�[�͈͂̐ݒ�
Render Render::withRenderArea(const uint32_t& width, const uint32_t& height)
{
	property.info.renderArea.extent = { width,height };

	return *this;
}

//�����_�[�摜�̏������F�̐ݒ�
Render Render::withClearColor(const glm::ivec4& color)
{
	property.isClear = true;

	for (int i = 0; i < 4; i++)
	{
		property.clearValues[0].color.int32[i] = color[i];
	}

	return *this;
}

//�����_�[�摜�̏������̃f�v�X�̐ݒ�
Render Render::withClearDepth(const float& depth)
{
	property.isClear = true;

	property.clearValues[1].depthStencil.depth = depth;

	return *this;
}

//�����_�[�摜�̏������̃X�e���V���̐ݒ�
Render Render::withClearStencil(const uint32_t& stencil)
{
	property.isClear = true;

	property.clearValues[1].depthStencil.stencil = stencil;

	return *this;
}

//�R�}���h��ςރR�}���h�o�b�t�@��ݒ�
Render Render::withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer)
{
	property.commandBuffer = commandBuffer;

	return *this;
}

//�v���p�e�B��Ԃ�
RenderProperty Render::Build()
{
	if (property.isClear)
	{
		property.info.clearValueCount = static_cast<uint32_t>(property.clearValues.size());
		property.info.pClearValues = property.clearValues.data();
	}

	return property;
}

//�����_�����O�J�n
void Render::RenderStart(const RenderProperty& property)
{
	vkCmdBeginRenderPass(property.commandBuffer->commandBuffer, &property.info
		, VK_SUBPASS_CONTENTS_INLINE);
}

//�����_�����O�I��
void Render::RenderEnd(const RenderProperty& property)
{
	vkCmdEndRenderPass(property.commandBuffer->commandBuffer);
}