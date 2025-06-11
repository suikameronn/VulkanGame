#include"UI.h"

#include"VulkanBase.h"
#include"GameManager.h"

UI::UI(std::shared_ptr<ImageData> image)
{
	layer = UILayer::BACKGROUND;

	initFrame = true;

	uiNum = UINum::IMAGE;

	exist = true;

	uiImage = image;

	GameManager* manager = GameManager::GetInstance();

	projMatrix = manager->getUIProjection();

	isUpdateTransformMatrix = true;

	position = glm::vec2(0.0f ,0.0f);

	rotate.z = 0.0f;

	scale = 1.0f;

	isVisible = true;

	vertices.resize(UIVertexCount);

	//UI�̉摜�𒣂�t���邽�߂̃|���S���̒��_�̍��W��ݒ�
	vertices[0].uv = glm::vec2(0.0f);
	vertices[1].uv = glm::vec2(1.0f, 0.0f);
	vertices[2].uv = glm::vec2(0.0f, 1.0f);
	vertices[3].uv = glm::vec2(1.0f);

	indices.resize(UIIndexCount);
	indices = { 0, 1, 2, 
				2, 3, 1 };

	aspect = glm::vec2(1.0f);

	uiWidth = 1.0f;
	uiHeight = 1.0f;

	//UI��\�����邽�߂̍s����L�^���邽�߂̃o�b�t�@���쐬
	VulkanBase::GetInstance()->uiCreateUniformBuffer(mappedBuffer);
}

//2D��̍��W��ݒ�
void UI::setPosition(glm::vec2 pos)
{
	position = pos;

	isUpdateTransformMatrix = true;
}

//2D��̉�]��ݒ�
void UI::setRotate(Rotate2D rot)
{
	rotate = rot;
	isUpdateTransformMatrix = true;
}

//2D��̃X�P�[����ݒ�
void UI::setScale(float scale)
{
	this->scale = scale;
	isUpdateTransformMatrix = true;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;
}

//UI�̕\���E��\����ݒ�
void UI::setVisible(bool visible)
{
	isVisible = visible;
}

//���_�z���Ԃ�
Vertex2D* UI::getVertices()
{
	return vertices.data();
}

//�C���f�b�N�X�z���Ԃ�
uint32_t* UI::getIndices()
{
	return indices.data();
}

//gpu��̒��_�o�b�t�@��Ԃ�
BufferObject* UI::getPointBuffer()
{
	return pointBuffers.data();
}

//ui�̉摜�̃e�N�X�`����Ԃ�
TextureData* UI::getUITexture()
{
	return uiImage->getTexture();
}

//ui�̉摜��Ԃ�
std::shared_ptr<ImageData> UI::getImageData()
{
	return uiImage;
}

//ui�̉摜��gpu��ɓW�J���A�e�N�X�`�����쐬
void UI::createTexture(std::shared_ptr<ImageData> image) 
{
	uiImage = image;

	VulkanBase* vulkan = VulkanBase::GetInstance();

	//ui�摜�̏c������v�Z
	float commonDivide = static_cast<float>(std::gcd(uiImage->getWidth(), uiImage->getHeight()));

	aspect[0] = uiImage->getWidth() / commonDivide;
	aspect[1] = uiImage->getHeight() / commonDivide;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;

	//ui�̏c���䂩��|���S���̍��W��ݒ�
	float zPos = (int)layer * 0.1f;
	vertices[0].pos = glm::vec3(0.0f, 0.0f, zPos);
	vertices[1].pos = glm::vec3(aspect[0], 0.0f, zPos);
	vertices[2].pos = glm::vec3(0.0f, aspect[1], zPos);
	vertices[3].pos = glm::vec3(aspect[0], aspect[1], zPos);
}

//ui��\��������W�ϊ��s����X�V
void UI::updateTransformMatrix()
{
	transformMatrix = glm::mat4(1.0f);

	//2D�p�̍s����쐬
	transformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f))
		* rotate.getRotateMatrix()
		* glm::translate(glm::mat4(1.0f), glm::vec3(-uiWidth / 2.0f, -uiHeight / 2.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	isUpdateTransformMatrix = false;
}

//ui�̍��W�ϊ��s����X�V
void UI::initFrameSettings()
{
	initFrame = false;

	//UI�̉摜��gpu��ɓW�J
	createTexture(uiImage);

	updateTransformMatrix();
}

//gpu��̃o�b�t�@��j��
void UI::cleanupVulkan()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	for (BufferObject& pointBuffer : pointBuffers)
	{
		vulkan->addDefferedDestructBuffer(pointBuffer);
	}
	vulkan->addDefferedDestructBuffer(mappedBuffer);
}

void UI::updateUniformBuffer()
{
	MatricesUBO2D ubo{};
	ubo.transformMatrix = transformMatrix;
	ubo.projection = projMatrix;

	memcpy(mappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
}

void UI::frameEnd()
{
	updateUniformBuffer();
}