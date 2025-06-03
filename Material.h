#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include<iostream>
#include<vector>
#include<algorithm>

#include<vulkan/vulkan.h>
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include"Object.h"

#include"ft2build.h"
#include FT_FREETYPE_H

//gpu��̃e�N�X�`���f�[�^
struct TextureData
{
	//�e�N�X�`���݂̂��Ճ��x��
	uint32_t mipLevel;
	//�e�N�X�`���̉摜�f�[�^
	VkImage image;
	VkDeviceMemory memory;
	//�摜�̃r���[
	VkImageView view;
	//�摜�̃T���v���[ �e�N�X�`���̋��E�̐ݒ�Ȃ�
	VkSampler sampler;

	void destroy(VkDevice& device)//�e��f�[�^�̔j��
	{
		vkDestroyImageView(device, view, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, memory, nullptr);
		vkDestroySampler(device, sampler, nullptr);

		delete this;
	}
};

//�摜�̃f�[�^��S���N���X 4�`�����l����p
class ImageData
{
private:
	//�摜�̕�
	int width;
	//�摜�̍���
	int height;
	//�摜�̌��̃`�����l����
	int texChannels;
	//�s�N�Z���̐F�̈ꎟ���z��
	std::vector<unsigned char> pixels;
	//HDRI�摜�p�̃s�N�Z���z��
	std::vector<float> hdriPixels;

	//�e�N�X�`��
	TextureData* texture;

public:

	ImageData(int width, int height,
		int channels,unsigned char* srcPixels)
	{
		this->width = width;
		this->height = height;
		this->texChannels = channels;

		texture = new TextureData();

		if (channels == 4)//�摜�ɓ����x�̃`�����l��������ꍇ�́A���̂܂�
		{
			this->pixels.resize(width * height * channels);
			std::fill(this->pixels.begin(), this->pixels.end(), 255);

			this->pixels.assign(srcPixels, srcPixels + ((width * height * channels) - 1));
		}
		else if (channels == 3)//�摜�ɓ����x�̃`�����l���������ꍇ�́A���ׂẴs�N�Z���̓����x��255�Ƃ���
							  //�����Ƀ`�����l����4�� gpu�ł̃e�N�X�`���쐬�̓s����
		{
			this->pixels.resize(width * height * channels);
			std::fill(this->pixels.begin(), this->pixels.end(), 255);

			for (int i = 0; i < width * height; i++)
			{
				pixels[i * 4] = srcPixels[i * 3];
				pixels[i * 4 + 1] = srcPixels[i * 3 + 1];
				pixels[i * 4 + 2] = srcPixels[i * 3 + 2];
			}
		}
		else if(channels == 1)
		{
			this->pixels.resize(width * height * channels);

			std::copy(srcPixels, srcPixels + (width * height), pixels.data());
		}
	}

	//�`�����l������4�ɌŒ�
	ImageData(int width, int height,
		int channels, float* srcPixels)
	{
		this->width = width;
		this->height = height;
		this->texChannels = 4;
		this->hdriPixels.resize(width * height * 4);
		std::fill(this->hdriPixels.begin(), this->hdriPixels.end(), 255.0f);

		texture = new TextureData();

		if (channels == 4)//�摜�ɓ����x�̃`�����l��������ꍇ�́A���̂܂�
		{
			this->hdriPixels.assign(srcPixels, srcPixels + ((width * height * channels) - 1));
		}
		else if (channels == 3)//�摜�ɓ����x�̃`�����l���������ꍇ�́A���ׂẴs�N�Z���̓����x��255�Ƃ���
			//�����Ƀ`�����l����4�� gpu�ł̃e�N�X�`���쐬�̓s����
		{
			for (int i = 0; i < width * height; i++)
			{
				hdriPixels[i * 4] = srcPixels[i * 3];
				hdriPixels[i * 4 + 1] = srcPixels[i * 3 + 1];
				hdriPixels[i * 4 + 2] = srcPixels[i * 3 + 2];
			}
		}
	}

	ImageData(FT_Bitmap& bitmap)
	{
		width = bitmap.width;//��
		height = bitmap.rows;//����
		texChannels = 1;

		texture = nullptr;
		
		pixels.resize(width * height);

		for (int i = 0; i < width * height; i++)
		{
			pixels[i] = bitmap.buffer[i];
		}
	}

	int getWidth()
	{
		return this->width;
	}

	int getHeight()
	{
		return this->height;
	}

	int getTexChannels()
	{
		return this->texChannels;
	}

	size_t getPixelPerByte()
	{
		if (hdriPixels.size() == 0)
		{
			return sizeof(unsigned char);
		}
		else
		{
			return sizeof(float);
		}
	}

	void* getPixelsData()
	{
		if (hdriPixels.size() == 0)
		{
			return pixels.data();
		}
		else
		{
			return hdriPixels.data();
		}
	}

	TextureData* getTexture()
	{
		return texture;
	}

	void cleanUpVulkan(VkDevice& device)
	{
		texture->destroy(device);
	}
};

//�V�F�[�_��̃e�N�X�`���̎Q�Ɨp�ԍ�
//�}�e���A�������̃e�N�X�`���������Ȃ��ꍇ�́A-1���L�^�����
struct TexCoordSets {
	uint8_t baseColor = 0;
	uint8_t metallicRoughness = 0;
	uint8_t specularGlossiness = 0;
	uint8_t normal = 0;
	uint8_t occlusion = 0;
	uint8_t emissive = 0;
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
};

//gltf���f���̃}�e���A��
class Material
{
public:

	Material()
	{
		baseColorTextureIndex = -1;
		metallicRoughnessTextureIndex = -1;
		normalTextureIndex = -1;
		occlusionTextureIndex = -1;
		emissiveTextureIndex = -1;
	}

	enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
	AlphaMode alphaMode = ALPHAMODE_OPAQUE;
	//�����x�̍ő�l�̐ݒ�
	float alphaCutoff = 1.0f;
	//���^���b�N�̏����l
	float metallicFactor = 1.0f;
	//�e���̏����l
	float roughnessFactor = 1.0f;
	//��{�F�̐ݒ�
	glm::vec4 baseColorFactor = glm::vec4(1.0f);
	//����
	glm::vec4 emissiveFactor = glm::vec4(0.0f);
	//��{�F�̃e�N�X�`���̔ԍ�
	int baseColorTextureIndex;
	//���^���b�N�Ƒe���̃e�N�X�`���̔ԍ�
	//�e����g�A���^���b�N��b�`�����l���Ɋi�[
	int metallicRoughnessTextureIndex;
	//�@���p�e�N�X�`��
	int normalTextureIndex;
	//�I�N���[�W�����������O�p�e�N�X�`���ԍ�
	int occlusionTextureIndex;
	//�����p�e�N�X�`���ԍ�
	int emissiveTextureIndex;
	bool doubleSided = false;

	//gltf���f�����̃}�e���A���̔ԍ�
	//gltf���f���̃��b�V�������}�e���A�����Q�Ƃ���ۂɎg�p
	int index = 0;
	bool unlit = false;
	float emissiveStrength = 1.0f;

	//gpu��̃}�e���A���Ɋւ���f�[�^��R�Â������
	//�����_�����O���Ɏg�p
	VkDescriptorSet descriptorSet;
	//�e�N�X�`�����]��uv�̎�ނ��L�^,-1�̏ꍇ�͂��̃e�N�X�`���������Ȃ����Ƃ�����
	TexCoordSets texCoordSets;

	//�e�N�X�`���ɑ΂���W��
	ShaderMaterial shaderMaterial;
	//gpu��̃e�N�X�`���ɑ΂���W���p�̃o�b�t�@
	MappedBuffer sMaterialMappedBuffer;

	void setupShaderMaterial()
	{
		shaderMaterial.emissiveFactor = emissiveFactor;

		//���ꂼ���uv���W�̃C���f�b�N�X��ݒ肵�Ă��� -1�̏ꍇ�́A���̃}�e���A���͂��̃e�N�X�`���������Ȃ�
		shaderMaterial.colorTextureSet = baseColorTextureIndex != -1 ? texCoordSets.baseColor : -1;
		shaderMaterial.normalTextureSet = normalTextureIndex != -1 ? texCoordSets.normal : -1;
		shaderMaterial.occlusionTextureSet = occlusionTextureIndex != -1 ? texCoordSets.occlusion : -1;
		shaderMaterial.emissiveTextureSet = emissiveTextureIndex != -1 ? texCoordSets.emissive : -1;
		shaderMaterial.alphaMask = static_cast<float>(alphaMode == AlphaMode::ALPHAMODE_MASK);
		shaderMaterial.alphaMaskCutoff = alphaCutoff;
		shaderMaterial.emissiveStrength = emissiveStrength;

		shaderMaterial.baseColorFactor = baseColorFactor;
		shaderMaterial.metallicFactor = metallicFactor;
		shaderMaterial.roughnessFactor = roughnessFactor;
		shaderMaterial.PhysicalDescriptorTextureSet = metallicRoughnessTextureIndex != -1 ? texCoordSets.metallicRoughness : -1;
	}
};