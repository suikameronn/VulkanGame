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

//行列を記録するユニフォームバッファを作成する
void TransformComponent::createUniformBuffer()
{
	VkDeviceSize size = sizeof(Transform);

	matBuffer = bufferFactory->Create(size, &transform
		, BufferUsage::UNIFORM, BufferTransferType::DST);
}

//ユニフォームバッファを示すディスクリプタセットを作成する
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

	//vkUpdate済み
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

//コンポーネントをアタッチした時点で実行
void TransformComponent::OnAwake()
{
	//バッファを作成
	createUniformBuffer();

	//バッファとディスクリプタセットを結び付ける
	createDescriptorSet();
}

//フレーム終了時に実行
void TransformComponent::OnFrameEnd()
{
	transform.modelMatrix = transformMat();
	transform.uvScale = scale;

	memcpy(matBuffer->mappedPtr, &transform, sizeof(Transform));
}