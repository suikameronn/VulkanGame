#include"Material.h"

//シェーダー上のマテリアルデータを記録するバッファを作成
void Material::createBuffer()
{
	shaderMaterialBuffer = bufferFactory->Create(sizeof(ShaderMaterial)
		, &shaderMaterial, BufferUsage::UNIFORM, BufferTransferType::NONE);
}

//ディスクリプタセットを作成
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