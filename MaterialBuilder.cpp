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
MaterialBuilder MaterialBuilder::initProperty()
{
	property.initProperty();

	return *this;
}

//��{�F�̃e�N�X�`����ݒ�
MaterialBuilder MaterialBuilder::withBaseColorTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.baseColor = uvIndex;
	property.baseColorTexture = texture;

	return *this;
}

//���^���b�N�Ƒe���̃e�N�X�`����ݒ�
MaterialBuilder MaterialBuilder::withMetallicRoughnessTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.metallicRoughness = uvIndex;
	property.metallicRoughnessTexture = texture;

	return *this;
}

//�@���p�e�N�X�`����ݒ�
MaterialBuilder MaterialBuilder::withNormalTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.normal = uvIndex;
	property.normalTexture = texture;

	return *this;
}

//�I�N���[�W�����������O�p�e�N�X�`����ݒ�
MaterialBuilder MaterialBuilder::withOcclusionTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.occlusion = uvIndex;
	property.occlusionTexture = texture;

	return *this;
}

//�����p�e�N�X�`����ݒ�
MaterialBuilder MaterialBuilder::withEmissiveTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.emissive= uvIndex;
	property.emissiveTexture = texture;

	return *this;
}

//��{�F�̐ݒ�
MaterialBuilder MaterialBuilder::withBaseColorFactor(const glm::vec4& factor)
{
	property.baseColorFactor = factor;

	return *this;
}

//�����̐ݒ�
MaterialBuilder MaterialBuilder::withEmissiveFactor(const glm::vec4& factor)
{
	property.emissiveFactor = factor;

	return *this;
}

//��������̓x������ݒ�
MaterialBuilder MaterialBuilder::withMetallicFactor(const float& factor)
{
	property.metallicFactor = factor;

	return *this;
}

//�e���̓x������ݒ�
MaterialBuilder MaterialBuilder::withRoughnessFactor(const float& factor)
{
	property.roughnessFactor = factor;

	return *this;
}

//�����x�̍ő�l��ݒ�
MaterialBuilder MaterialBuilder::withAlphaMaskCutOff(const float& cutoff)
{
	property.alphaMaskCutoff = cutoff;

	return *this;
}

//���ȕ��˂̋�����ݒ�
MaterialBuilder MaterialBuilder::withEmissiveStrength(const float& strength)
{
	property.emissiveStrength = strength;

	return *this;
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
	std::shared_ptr<Material> material = std::make_shared<Material>(property);

	return material;
}