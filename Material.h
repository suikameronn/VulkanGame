#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include<iostream>
#include<vector>
#include<algorithm>

#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

#include"Object.h"

//�V�F�[�_��̃e�N�X�`���̎Q�Ɨp�ԍ�
//�}�e���A�������̃e�N�X�`���������Ȃ��ꍇ�́A-1���L�^�����
struct TexCoordSets 
{
	int baseColor;
	int metallicRoughness;
	int specularGlossiness;
	int normal;
	int occlusion;
	int emissive;

	void init()
	{
		baseColor = -1;
		metallicRoughness = -1;
		specularGlossiness = -1;
		normal = -1;
		occlusion = -1;
		emissive = -1;
	};
};

struct MaterialProperty
{
	TexCoordSets texCoordSet;

	std::shared_ptr<Texture> baseColorTexture;
	std::shared_ptr<Texture> metallicRoughnessTexture;
	std::shared_ptr<Texture> normalTexture;
	std::shared_ptr<Texture> occlusionTexture;
	std::shared_ptr<Texture> emissiveTexture;

	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;

	float metallicFactor;
	float roughnessFactor;
	float alphaMaskCutoff;
	float emissiveStrength;

	void initProperty()
	{
		texCoordSet.init();

		//�����x�̍ő�l�̐ݒ�
		alphaMaskCutoff = 1.0f;
		//���^���b�N�̏����l
		metallicFactor = 1.0f;
		//�e���̏����l
		roughnessFactor = 1.0f;
		//���ȕ��˂̋���
		emissiveStrength = 1.0f;
		//��{�F�̐ݒ�
		baseColorFactor = glm::vec4(1.0f);
		//����
		emissiveFactor = glm::vec4(0.0f);

		baseColorTexture = nullptr;
		metallicRoughnessTexture = nullptr;
		normalTexture = nullptr;
		occlusionTexture = nullptr;
		emissiveTexture = nullptr;
	}

	void Build(std::shared_ptr<Texture> emptyTexture)
	{
		if (!baseColorTexture)
		{
			baseColorTexture = emptyTexture;
		}

		if (!metallicRoughnessTexture)
		{
			metallicRoughnessTexture = emptyTexture;
		}

		if (!normalTexture)
		{
			normalTexture = emptyTexture;
		}

		if (!occlusionTexture)
		{
			occlusionTexture = emptyTexture;
		}

		if (!emissiveTexture)
		{
			emissiveTexture = emptyTexture;
		}
	}
};

//�V�F�[�_��ŃA�N�Z�X�邽�߂̃}�e���A��
//�e�N�X�`���ɑ΂���W�������C��
struct ShaderMaterial
{
	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;
	glm::vec4 diffuseFactor;
	glm::vec4 specularFactor;
	int colorTextureSet;
	int PhysicalDescriptorTextureSet;
	int normalTextureSet;
	int occlusionTextureSet;
	int emissiveTextureSet;
	float metallicFactor;
	float roughnessFactor;
	float alphaMask;
	float alphaMaskCutoff;
	float emissiveStrength;

	void init(const MaterialProperty& prop)
	{
		baseColorFactor = prop.baseColorFactor;
		emissiveFactor = prop.emissiveFactor;
		diffuseFactor = glm::vec4(1.0f);
		specularFactor = glm::vec4(1.0f);
		
		colorTextureSet = prop.texCoordSet.baseColor;
		PhysicalDescriptorTextureSet = prop.texCoordSet.metallicRoughness;
		normalTextureSet = prop.texCoordSet.normal;
		occlusionTextureSet = prop.texCoordSet.occlusion;
		emissiveTextureSet = prop.texCoordSet.emissive;
		metallicFactor = prop.metallicFactor;
		roughnessFactor = prop.roughnessFactor;
		alphaMaskCutoff = prop.alphaMaskCutoff;
		alphaMask = 1.0f - alphaMaskCutoff; //�����x�̍ő�l
		emissiveStrength = prop.emissiveStrength;
	}
};

//gltf���f���̃}�e���A��
class Material
{
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//gpu��̃e�N�X�`���ɑ΂���W���p�̃o�b�t�@
	std::shared_ptr<GpuBuffer> shaderMaterialBuffer;

	//gpu��̃}�e���A���Ɋւ���f�[�^��R�Â������
	//�����_�����O���Ɏg�p
	std::shared_ptr<DescriptorSet> descriptorSet;

	//�V�F�[�_�[��̃}�e���A���f�[�^���L�^����o�b�t�@���쐬
	void createBuffer();
	//�f�B�X�N���v�^�Z�b�g���쐬
	void createDescriptorSet();

public:

	Material(const MaterialProperty& prop, std::shared_ptr<GpuBufferFactory> bf,
		std::shared_ptr<DescriptorSetLayoutFactory> lf, std::shared_ptr<DescriptorSetFactory> df)
	{
		bufferFactory = bf;
		layoutFactory = lf;
		descriptorSetFactory = df;

		shaderMaterial.init(prop);
		texCoordSets = prop.texCoordSet;

		//gpu���\�[�X�̍쐬
		createBuffer();
		createDescriptorSet();

		//�X�V�����\���̂̒l��gpu��̃o�b�t�@�ɃR�s�[
		bufferFactory->copyMemory(
			sizeof(ShaderMaterial),
			&shaderMaterial,
			shaderMaterialBuffer
		);
	}

	enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
	AlphaMode alphaMode = ALPHAMODE_OPAQUE;

	//gltf���f�����̃}�e���A���̔ԍ�
	//gltf���f���̃��b�V�������}�e���A�����Q�Ƃ���ۂɎg�p
	int index = 0;

	//�e�N�X�`�����]��uv�̎�ނ��L�^,-1�̏ꍇ�͂��̃e�N�X�`���������Ȃ����Ƃ�����
	TexCoordSets texCoordSets;

	//�e�N�X�`���ɑ΂���W��
	ShaderMaterial shaderMaterial;

	std::shared_ptr<DescriptorSet> getDescriptorSet() const
	{
		return descriptorSet;
	}

	std::shared_ptr<GpuBuffer> getShaderMaterialBuffer() const
	{
		return shaderMaterialBuffer;
	}	
};