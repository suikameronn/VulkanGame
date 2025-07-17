#include"TransformComponent.h"

TransformComponent::TransformComponent(std::shared_ptr<GpuBufferFactory> buffer
	, std::shared_ptr<DescriptorSetFactory> desc
	, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout)
{
	bufferFactory = buffer;

	descriptorSetFactory = desc;

	layoutFactory = layout;

	pivot = glm::vec3(0.0f);

	position = glm::vec3(0.0f);
	rotateDeg = glm::vec3(0.0f);
	scale = glm::vec3(1.0f);
}

//�s����L�^���郆�j�t�H�[���o�b�t�@���쐬����
void TransformComponent::createUniformBuffer()
{
	VkDeviceSize size = sizeof(Transform);

	matBuffer = bufferFactory->Create(size, &transform
		, BufferUsage::UNIFORM, BufferTransferType::DST);
}

//���j�t�H�[���o�b�t�@�������f�B�X�N���v�^�Z�b�g���쐬����
void TransformComponent::createDescriptorSet()
{
	std::shared_ptr<DescriptorSetLayout> layout = layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT);

	DescriptorSetProperty property = descriptorSetFactory->getBuilder()
		->withBindingBuffer(0)
		.withBuffer(matBuffer)
		.withDescriptorSetCount(1)
		.withDescriptorSetLayout(layout)
		.withOffset(0)
		.withRange(sizeof(Transform))
		.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.addBufferInfo()
		.Build();

	//vkUpdate�ς�
	matBufferBindDesc = descriptorSetFactory->Create(property);
}

glm::mat4 TransformComponent::tlanslateMat()
{
	return glm::translate(glm::mat4(1.0f), position);
}

glm::mat4 TransformComponent::rotateMat()
{
	glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(rotateDeg.x), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(rotateDeg.y), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotateDeg.z), glm::vec3(0.0f, 0.0f, 1.0f));
	
	return rotY * rotX * rotZ;
}

glm::mat4 TransformComponent::scaleMat()
{
	return glm::scale(glm::mat4(1.0f), scale);
}

glm::mat4 TransformComponent::transformMat()
{
	return tlanslateMat()* rotateMat()* scaleMat();
}

//�R���|�[�l���g���A�^�b�`�������_�Ŏ��s
void TransformComponent::OnAwake()
{
	//�o�b�t�@���쐬
	createUniformBuffer();

	//�o�b�t�@�ƃf�B�X�N���v�^�Z�b�g�����ѕt����
	createDescriptorSet();
}

//�t���[���I�����Ɏ��s
void TransformComponent::OnFrameEnd()
{
	transform.modelMatrix = transformMat();
	transform.uvScale = scale;

	memcpy(matBuffer->mappedPtr, &transform, sizeof(Transform));
}