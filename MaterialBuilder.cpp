#include"MaterialBuilder.h"

void MaterialBuilder::createEmptyTexture()
{
	const uint32_t texChannel = 4;
	const uint32_t size = 2;
	const std::vector<unsigned char> pixels(size * size, 0);

	emptyTexture = textureFactory->Create(texChannel,pixels.data()
		, size, size, TexturePattern::NORMAL);
}

//�}�e���A���̃v���p�e�B��������
std::shared_ptr<MaterialBuilder> MaterialBuilder::initProperty()
{
	property.initProperty();

	return shared_from_this();
}

//��{�F�̃e�N�X�`����ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withBaseColorTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.baseColor = uvIndex;
	property.baseColorTexture = texture;

	return shared_from_this();
}

//���^���b�N�Ƒe���̃e�N�X�`����ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withMetallicRoughnessTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.metallicRoughness = uvIndex;
	property.metallicRoughnessTexture = texture;

	return shared_from_this();
}

//�@���p�e�N�X�`����ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withNormalTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.normal = uvIndex;
	property.normalTexture = texture;

	return shared_from_this();
}

//�I�N���[�W�����������O�p�e�N�X�`����ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withOcclusionTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.occlusion = uvIndex;
	property.occlusionTexture = texture;

	return shared_from_this();
}

//�����p�e�N�X�`����ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withEmissiveTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.emissive= uvIndex;
	property.emissiveTexture = texture;

	return shared_from_this();
}

//��{�F�̐ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withBaseColorFactor(const glm::vec4& factor)
{
	property.baseColorFactor = factor;

	return shared_from_this();
}

//�����̐ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withEmissiveFactor(const glm::vec4& factor)
{
	property.emissiveFactor = factor;

	return shared_from_this();
}

//��������̓x������ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withMetallicFactor(const float& factor)
{
	property.metallicFactor = factor;

	return shared_from_this();
}

//�e���̓x������ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withRoughnessFactor(const float& factor)
{
	property.roughnessFactor = factor;

	return shared_from_this();
}

//�����x�̍ő�l��ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withAlphaMaskCutOff(const float& cutoff)
{
	property.alphaMaskCutoff = cutoff;

	return shared_from_this();
}

//���ȕ��˂̋�����ݒ�
std::shared_ptr<MaterialBuilder> MaterialBuilder::withEmissiveStrength(const float& strength)
{
	property.emissiveStrength = strength;

	return shared_from_this();
}

//�}�e���A���̃v���p�e�B���쐬
MaterialProperty MaterialBuilder::Build()
{
	property.Build(emptyTexture);

	MaterialProperty prop = property;

	initProperty();

	return prop;
}

//�}�e���A�����쐬
std::shared_ptr<Material> MaterialBuilder::Create(const MaterialProperty& property)
{
	std::shared_ptr<Material> material = std::make_shared<Material>(property, bufferFactory, layoutFactory, descriptorSetFactory);

	return material;
}