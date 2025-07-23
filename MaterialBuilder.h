#pragma once

#include"Material.h"

#include"TextureFactory.h"

class MaterialBuilder
{
private:

	//�t�@�N�g���[�N���X
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;
	std::shared_ptr<TextureFactory> textureFactory;

	std::shared_ptr<Texture> emptyTexture;

	MaterialProperty property;

	void createEmptyTexture();

public:

	MaterialBuilder(std::shared_ptr<GpuBufferFactory> buffer,
		std::shared_ptr<DescriptorSetLayoutFactory> layout,
		std::shared_ptr<DescriptorSetFactory> desc
		, std::shared_ptr<TextureFactory> texture)
	{
		bufferFactory = buffer;
		layoutFactory = layout;
		descriptorSetFactory = desc;
		textureFactory = texture;

		property.initProperty();

		//�_�~�[�e�N�X�`�������
		createEmptyTexture();
	}

	//�}�e���A���̃v���p�e�B��������
	MaterialBuilder initProperty();

	//��{�F�̃e�N�X�`����ݒ�
	MaterialBuilder withBaseColorTexture(const int& uvIndex,std::shared_ptr<Texture> texture);

	//���^���b�N�Ƒe���̃e�N�X�`����ݒ�
	MaterialBuilder withMetallicRoughnessTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//�@���p�e�N�X�`����ݒ�
	MaterialBuilder withNormalTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//�I�N���[�W�����������O�p�e�N�X�`����ݒ�
	MaterialBuilder withOcclusionTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//�����p�e�N�X�`����ݒ�
	MaterialBuilder withEmissiveTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//��{�F�̐ݒ�
	MaterialBuilder withBaseColorFactor(const glm::vec4& factor);

	//�����̐ݒ�
	MaterialBuilder withEmissiveFactor(const glm::vec4& factor);

	//��������̓x������ݒ�
	MaterialBuilder withMetallicFactor(const float& factor);

	//�e���̓x������ݒ�
	MaterialBuilder withRoughnessFactor(const float& factor);

	//�����x�̍ő�l��ݒ�
	MaterialBuilder withAlphaMaskCutOff(const float& cutoff);

	//���ȕ��˂̋�����ݒ�
	MaterialBuilder withEmissiveStrength(const float& strength);

	//�}�e���A���̃v���p�e�B���쐬
	MaterialProperty Build();

	//�}�e���A�����쐬
	std::shared_ptr<Material> Create(const MaterialProperty& property);
};