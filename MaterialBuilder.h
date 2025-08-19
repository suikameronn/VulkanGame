#pragma once

#include"Material.h"

#include"TextureFactory.h"

class MaterialBuilder : public std::enable_shared_from_this<MaterialBuilder>
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

	~MaterialBuilder()
	{
#ifdef _DEBUG
		std::cout << "MaterialBuilder :: �f�X�g���N�^" << std::endl;
#endif
	}

	//�}�e���A���̃v���p�e�B��������
	std::shared_ptr<MaterialBuilder> initProperty();

	//��{�F�̃e�N�X�`����ݒ�
	std::shared_ptr<MaterialBuilder> withBaseColorTexture(const int& uvIndex,std::shared_ptr<Texture> texture);

	//���^���b�N�Ƒe���̃e�N�X�`����ݒ�
	std::shared_ptr<MaterialBuilder> withMetallicRoughnessTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//�@���p�e�N�X�`����ݒ�
	std::shared_ptr<MaterialBuilder> withNormalTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//�I�N���[�W�����������O�p�e�N�X�`����ݒ�
	std::shared_ptr<MaterialBuilder> withOcclusionTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//�����p�e�N�X�`����ݒ�
	std::shared_ptr<MaterialBuilder> withEmissiveTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//��{�F�̐ݒ�
	std::shared_ptr<MaterialBuilder> withBaseColorFactor(const glm::vec4& factor);

	//�����̐ݒ�
	std::shared_ptr<MaterialBuilder> withEmissiveFactor(const glm::vec4& factor);

	//��������̓x������ݒ�
	std::shared_ptr<MaterialBuilder> withMetallicFactor(const float& factor);

	//�e���̓x������ݒ�
	std::shared_ptr<MaterialBuilder> withRoughnessFactor(const float& factor);

	//�����x�̍ő�l��ݒ�
	std::shared_ptr<MaterialBuilder> withAlphaMaskCutOff(const float& cutoff);

	//���ȕ��˂̋�����ݒ�
	std::shared_ptr<MaterialBuilder> withEmissiveStrength(const float& strength);

	//�}�e���A���̃v���p�e�B���쐬
	MaterialProperty Build();

	//�}�e���A�����쐬
	std::shared_ptr<Material> Create(const MaterialProperty& property);
};