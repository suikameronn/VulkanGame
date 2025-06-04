#include"UI.h"

#include"VulkanBase.h"
#include"GameManager.h"

UI::UI(std::shared_ptr<ImageData> image)
{
	uiNum = UINum::UI;

	exist = true;

	GameManager* manager = GameManager::GetInstance();

	projMatrix = manager->getUIProjection();

	isUpdateTransformMatrix = true;

	position = glm::vec2(0.0f ,0.0f);

	rotate.z = 0.0f;

	scale = 1.0f;

	isVisible = true;

	vertices = { glm::vec2(0.0f) };

	//�����`�̒��_��uv��ݒ肷��
	vertices[0].uv = glm::vec2(0.0f);
	vertices[1].uv = glm::vec2(1.0f, 0.0f);
	vertices[2].uv = glm::vec2(0.0f, 1.0f);
	vertices[3].uv = glm::vec2(1.0f);

	indices = { 0, 1, 2, // �ŏ��̎O�p�`
				2, 3, 1 };  // 2�Ԗڂ̎O�p�`

	aspect = glm::vec2(1.0f);

	uiWidth = 1.0f;
	uiHeight = 1.0f;

	//���W�ϊ��s��p�̃o�b�t�@�̍쐬
	VulkanBase::GetInstance()->uiCreateUniformBuffer(mappedBuffer);

	//�e�N�X�`���̐ݒ�
	createTexture(image);
}

//���W�̐ݒ�
void UI::setPosition(glm::vec2 pos)
{
	position = pos;

	isUpdateTransformMatrix = true;
}

//��]�̐ݒ�
void UI::setRotate(Rotate2D rot)
{
	rotate = rot;
	isUpdateTransformMatrix = true;
}

//�L�k�̐ݒ�
void UI::setScale(float scale)
{
	this->scale = scale;
	isUpdateTransformMatrix = true;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;
}

//�\����\���̐ݒ� �����ɂ���Đݒ�
void UI::setVisible(bool visible)
{
	isVisible = visible;
}

//���_�z��̎擾
Vertex2D* UI::getVertices()
{
	return vertices.data();
}

//�C���f�b�N�X�z��̎擾
uint32_t* UI::getIndices()
{
	return indices.data();
}

//gpu��̒��_�Ɋւ���o�b�t�@���擾
BufferObject& UI::getPointBuffer()
{
	return pointBuffer;
}

//ui�Ƃ��Č�����e�N�X�`���̎擾
TextureData* UI::getUITexture()
{
	return uiImage->getTexture();
}

//ui�摜��Ԃ�
std::shared_ptr<ImageData> UI::getImageData()
{
	return uiImage;
}

//�e�N�X�`���摜�̐ݒ�ƃe�N�X�`���̍쐬
void UI::createTexture(std::shared_ptr<ImageData> image) 
{
	uiImage = image;

	VulkanBase* vulkan = VulkanBase::GetInstance();

	//VkDescriptorSet�̗p�ӂ��o���Ă���Ȃ�A���ѕt�����X�V����
	if (descriptorSet)
	{
		VulkanBase::GetInstance()->createUIDescriptorSet(uiImage->getTexture(), mappedBuffer, descriptorSet);
	}

	//�摜�̃A�X�y�N�g����v�Z����
	float commonDivide = static_cast<float>(std::gcd(uiImage->getWidth(), uiImage->getHeight()));

	aspect[0] = uiImage->getWidth() / commonDivide;
	aspect[1] = uiImage->getHeight() / commonDivide;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;

	//���_���A�X�y�N�g��ɍ��킹�čX�V����
	vertices[0].pos = glm::vec2(0.0f,0.0f);
	vertices[1].pos = glm::vec2(aspect[0], 0.0f);
	vertices[2].pos = glm::vec2(0.0f, aspect[1]);
	vertices[3].pos = glm::vec2(aspect[0], aspect[1]);
}

//���W�ϊ��s��̍X�V
void UI::updateTransformMatrix()
{
	transformMatrix = glm::mat4(1.0f);

	//�摜�𒆐S�ɉ�]������
	transformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f))
		* rotate.getRotateMatrix()
		* glm::translate(glm::mat4(1.0f), glm::vec3(-uiWidth / 2.0f, -uiHeight / 2.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	isUpdateTransformMatrix = false;
}

//�����t���[���̎��Ɏ��s����
void UI::initFrameSettings()
{
	updateTransformMatrix();
}

void UI::cleanupVulkan()
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	vkDestroyBuffer(device, pointBuffer.vertBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.vertHandler, nullptr);

	vkDestroyBuffer(device, pointBuffer.indeBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.indeHandler, nullptr);

	//uniform buffer�̉��
	mappedBuffer.destroy(device);
}

//���j�t�H�[���o�b�t�@�̍X�V
void UI::updateUniformBuffer()
{
	MatricesUBO2D ubo{};
	ubo.transformMatrix = transformMatrix;
	ubo.projection = projMatrix;

	memcpy(mappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
}

//�t���[���I�����Ɏ��s
void UI::frameEnd()
{
	updateUniformBuffer();
}