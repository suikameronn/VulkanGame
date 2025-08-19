#include"Material.h"

//シェーダー上のマテリアルデータを記録するバッファを作成
void Material::createBuffer()
{
	shaderMaterialBuffer = bufferFactory->Create(sizeof(ShaderMaterial)
		, &shaderMaterial, BufferUsage::UNIFORM, BufferTransferType::NONE);
}

//ディスクリプタセットを作成
void Material::createDescriptorSet(const MaterialProperty& prop)
{
	const std::shared_ptr<DescriptorSetLayout> layout =
		layoutFactory->Create(LayoutPattern::MATERIAL);

	descriptorSet = descriptorSetFactory->Create
	(
		descriptorSetFactory->getBuilder()
		->initProperty()
		->withBindingBuffer(0)
		->withBuffer(shaderMaterialBuffer)
		->withDescriptorSetCount(1)
		->withDescriptorSetLayout(layout)
		->withRange(sizeof(ShaderMaterial))
		->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		->addBufferInfo()
		->withBindingImage(1)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->withTexture(prop.baseColorTexture)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->addImageInfo()
		->withBindingImage(2)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->withTexture(prop.metallicRoughnessTexture)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->addImageInfo()
		->withBindingImage(3)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->withTexture(prop.normalTexture)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->addImageInfo()
		->withBindingImage(4)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->withTexture(prop.occlusionTexture)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->addImageInfo()
		->withBindingImage(5)
		->withImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->withTexture(prop.emissiveTexture)
		->withTypeImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		->addImageInfo()
		->Build()
	);
}