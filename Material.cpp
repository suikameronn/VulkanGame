#include"Material.h"

//�V�F�[�_�[��̃}�e���A���f�[�^���L�^����o�b�t�@���쐬
void Material::createBuffer()
{
	shaderMaterialBuffer = bufferFactory->Create(sizeof(ShaderMaterial)
		, &shaderMaterial, BufferUsage::UNIFORM, BufferTransferType::NONE);
}

//�f�B�X�N���v�^�Z�b�g���쐬
void Material::createDescriptorSet()
{
	const std::shared_ptr<DescriptorSetLayout> layout =
		layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_FRAG);

	descriptorSet = descriptorSetFactory->Create(
		descriptorSetFactory->getBuilder()
		->withBindingBuffer(0)
		.withBuffer(shaderMaterialBuffer)
		.withDescriptorSetCount(1)
		.withDescriptorSetLayout(layout)
		.withOffset(0)
		.withRange(sizeof(ShaderMaterial))
		.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.addBufferInfo()
		.Build()
	);
}