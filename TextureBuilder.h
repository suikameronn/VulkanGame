#pragma once

#include<unordered_map>
#include<string>

#include"VulkanCore.h"
#include"GpuBufferFactory.h"

#include "tiny_gltf.h"

#define RGBA 4

struct Texture;

struct TextureImageProperty
{
	VkImageCreateInfo info;//VkImage�̍쐬���
	VkImageLayout finalLayout; //�ŏI�I�ȃe�N�X�`���̃��C�A�E�g
	VkMemoryPropertyFlags memProperty;

	size_t pixelSize;
	void* pixels;

	void initProperty()
	{
		info = VkImageCreateInfo{};

		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.extent.depth = 1;
		info.imageType = VK_IMAGE_TYPE_2D;

		finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		pixelSize = 0;

		if (!pixels)
		{
			delete[] pixels;
		}

		pixels = nullptr;
	}
};

struct TextureViewProperty
{
	VkImageViewCreateInfo info;

	void initProperty()
	{
		info = VkImageViewCreateInfo{};

		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.subresourceRange.baseArrayLayer = 0;
	}
};

struct TextureSamplerProperty
{
	VkSamplerCreateInfo info; //�T���v���[�̍쐬���

	void initProperty()
	{
		info = VkSamplerCreateInfo{};

		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.minLod = 0.0f;
		info.maxLod = 1.0f;
		info.mipLodBias = 0.0f;
		info.maxAnisotropy = 1.0f;
		info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	}
};

struct TextureProperty
{
	TextureImageProperty image;
	std::vector<TextureViewProperty> viewArray;
	TextureSamplerProperty sampler;

	void initProperty()
	{
		image.initProperty();

		viewArray.clear();

		sampler.initProperty();
	}
};

class TextureBuilder
{
private:

	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	std::shared_ptr<GpuBufferFactory> bufferFactory;

	//�e��v���p�e�B
	TextureProperty property;

	TextureImageProperty image;
	TextureViewProperty view;
	TextureSamplerProperty sampler;

	//vkCreateImage���Ăяo��
	void createVkImageAndMemory(const TextureImageProperty& info, VkImage& image, VkDeviceMemory& memory);

	//�o�b�t�@����VkImage�ɉ摜�f�[�^���R�s�[
	void copyBufferToImage(const std::shared_ptr<GpuBuffer> stagingBuffer, VkImage& image
		, const TextureImageProperty& imageProperty);

	//�~�b�v�}�b�v���x�����쐬
	void generateMipmaps(VkImage image, const TextureImageProperty& imageProperty);

	//gpu��ɉ摜�f�[�^��W�J
	void createImage(const size_t& pixelSize, const void* pixels
		, const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory);
	void createImage(const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory);

	//���̃e�N�X�`���̃r���[���쐬
	void createImageView(const TextureViewProperty& viewProperty, VkImage& image, VkImageView& view);

	//�T���v���[�̍쐬
	void createSampler(const TextureSamplerProperty& samplerProperty,VkSampler& sampler);

	//�O���̉摜��ǂݍ���
	void loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
		, unsigned char*& pixels);
	void loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
		, float*& pixels);

public:

	TextureBuilder(std::shared_ptr<VulkanCore> core,std::shared_ptr<GpuBufferFactory> buffer);

	//�v���p�e�B���擾����
	TextureProperty Build();

	void Create(const uint32_t& texChannel, const unsigned char* pixels, const TextureProperty& property
		, VkImage& image, VkDeviceMemory& memory, std::vector<VkImageView>& viewArray, VkSampler& sampler);
	
	void Create(const TextureProperty& property
		, VkImage& image, VkDeviceMemory& memory, std::vector<VkImageView>& viewArray, VkSampler& sampler);

	//�X���b�v�`�F�[���p
	void Create(const TextureProperty& property, VkImage& image, VkDeviceMemory& memory, std::vector<VkImageView>& viewArray);

	//�v���p�e�B�̏�����
	TextureBuilder& initProperty();

	//�e�N�X�`���T�C�Y�̐ݒ�
	TextureBuilder& withWidthHeight(const uint32_t& width, const uint32_t& height);
	TextureBuilder& withWidthHeight(const uint32_t& width, const uint32_t& height,const uint32_t& mipmapLevel);

	//�摜�t�@�C������s�N�Z����ǂݎ��
	TextureBuilder& withImageFile(const std::string filePath);
	TextureBuilder& withImageFile(const std::string filePath, const uint32_t miplevels);

	//�t�H�[�}�b�g��ݒ�
	TextureBuilder& withFormat(const VkFormat& format);

	//VkImageType��ݒ�
	TextureBuilder& withImageType(const VkImageType& type);

	//VkImageFlag��ݒ�(SamplerCube�̍쐬���ȂǂɎg�p)
	TextureBuilder& withImageFlag(const VkImageCreateFlagBits& flag);

	//�}���`�T���v�����O���̐ݒ�
	TextureBuilder& withNumSamples(const VkSampleCountFlagBits& numSamples);

	//�s�N�Z���̔z�u��ݒ�
	TextureBuilder& withTiling(const VkImageTiling& tiling);

	//�e�N�X�`���̃o�b�t�@�̎g������ݒ�
	TextureBuilder& withUsage(const VkImageUsageFlags& usage);

	//�������z�u��ݒ�
	TextureBuilder& withMemoryProperty(const VkMemoryPropertyFlags& prop);

	//�����̃e�N�X�`���̃��C�A�E�g
	TextureBuilder& withInitialLayout(const VkImageLayout& layout);

	//�ŏI�I�ȃe�N�X�`���̃��C�A�E�g
	TextureBuilder& withFinalLayout(const VkImageLayout& layout);

	//�e�N�X�`���̃��C���[����ݒ肷��
	TextureBuilder& withLayerCount(const uint32_t& layerCount);

	//�r���[�̃^�C�v��ݒ肷��
	TextureBuilder& withViewType(const VkImageViewType& type);

	//�r���[���A�N�Z�X�ł���f�[�^��ݒ�
	TextureBuilder& withViewAccess(const VkImageAspectFlags& flag);

	//�r���[���A�N�Z�X���郌�C���[�͈̔͂��w�肷��
	TextureBuilder& withTargetLayer(const uint32_t baseLayer,const uint32_t tagetLayerCount);

	//�r���[���A�N�Z�X����~�b�v�}�b�v���x�����w�肷��
	TextureBuilder& withTargetMipmapLevel(const uint32_t baseMipmapLevel, const uint32_t levelCount);

	//�r���[��ςݏグ��
	TextureBuilder& addView();

	//�T���v���[�̃~�b�v�}�b�v���x���Ԃ̕�ԕ��@��ݒ肷��
	TextureBuilder& withMipMapMode(const VkSamplerMipmapMode& mode);

	//�e�N�X�`���̋��E�����̏�����ݒ�
	TextureBuilder& withAddressMode(const VkSamplerAddressMode& mode);

	//�e�N�X�`���̊g�厞�̕�ԕ��@��ݒ肷��
	TextureBuilder& withMagFilter(const VkFilter& filter);

	//�e�N�X�`���̏k�����̕�ԕ��@��ݒ肷��
	TextureBuilder& withMinFilter(const VkFilter& filter);

	//�摜�̃��C�A�E�g��ύX����
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout
		, uint32_t mipLevels, uint32_t layerCount);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout
		, uint32_t mipLevels, uint32_t layerCount, std::shared_ptr<CommandBuffer> commandBuffer);
};