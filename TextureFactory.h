#pragma once

#include"TextureBuilder.h"

#include"TextureCopy.h"

enum class TexturePattern
{
	NORMAL,
	CUBEMAP,
	CALC_CUBEMAP
};

class TextureFactory : public std::enable_shared_from_this<TextureFactory>
{
private:

	uint32_t count;

	//�t���[���C���f�b�N�X
	uint32_t frameIndex;

	VkDevice device;

	//�r���_�[
	std::shared_ptr<TextureBuilder> builder;

	//�R�s�[
	std::shared_ptr<TextureCopy> copy;

	//�O���̉摜�t�@�C����ǂݍ��񂾏ꍇ�͂�����Texture���L�^����
	std::unordered_map<std::string, std::weak_ptr<Texture>> textureStorage;

	//�j���\�胊�X�g
	std::array<std::list<VkImage>, 2> destructListImage;
	std::array<std::list<VkDeviceMemory>, 2> destructListMemory;
	std::array<std::list<VkImageView>, 2> destructListView;
	std::array<std::list<VkSampler>, 2> destructListSampler;

	//����̃p�^�[������v���p�e�B�ɕϊ�
	TextureProperty convertPattern(const uint32_t& width, const uint32_t& height
		, const TexturePattern& pattern);
	//����̃p�^�[������v���p�e�B�ɕϊ�
	TextureProperty convertPattern(const TexturePattern& pattern);
	TextureProperty convertPattern(const std::string imageFilePath, const TexturePattern& pattern);

public:

	TextureFactory(VkDevice& d, std::shared_ptr<TextureBuilder> b);

	~TextureFactory()
	{
		for (int i = 0; i < 2; i++)
		{
			resourceDestruct();
			resourceDestruct();
		}

#ifdef _DEBUG
		std::cout << "TextureFactory :: �f�X�g���N�^" << std::endl;
#endif
	}

	//�r���_�[���擾
	std::shared_ptr<TextureBuilder> getBuilder()
	{
		return builder;
	}

	//�R�s�[���擾
	std::shared_ptr<TextureCopy> getCopy()
	{
		return copy;
	}

	//�v���Z�b�g�̃v���p�e�B���擾
	TextureProperty getPreset(const TexturePattern& pattern);

	//�O���̉摜�t�@�C������e�N�X�`�������ꍇ�𕪂���
	//����̃v���p�e�B���g��
	std::shared_ptr<Texture> Create(const std::string& filePath, const TexturePattern& pattern);

	//�摜�f�[�^��z���Ŏ󂯎��
	std::shared_ptr<Texture> Create(const uint32_t& texChannel, const unsigned char* pixels
		, const uint32_t& width, const uint32_t& height, const TexturePattern& pattern);

	//�摜�f�[�^�͓��ꂸ�ɁA�e�N�X�`���o�b�t�@�݂̂����
	std::shared_ptr<Texture> Create(const TextureProperty& property);
	std::shared_ptr<Texture> Create(const uint32_t& width, const uint32_t& height
		, const TexturePattern& pattern);

	std::shared_ptr<Texture> ImageViewCreate(const TextureProperty& property);
	std::shared_ptr<Texture> ImageCreate(const TextureProperty& property);
	std::shared_ptr<Texture> ViewCreate(const TextureProperty& property, VkImage& image);

	//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
	void addDefferedDestruct(VkImage& image, VkDeviceMemory& memory
		, std::vector<VkImageView>& viewArray, VkSampler& sampler);

	//���\�[�X��j������
	void resourceDestruct();
};

struct Texture
{
	uint32_t index;

	//�e�N�X�`���̉摜�f�[�^
	VkImage image;
	VkDeviceMemory memory;
	//�摜�̃r���[
	std::vector<VkImageView> viewArray;
	//�摜�̃T���v���[ �e�N�X�`���̋��E�̐ݒ�Ȃ�
	VkSampler sampler;

	//�e��v���p�e�B���L�^
	TextureProperty property;

	std::shared_ptr<TextureFactory> factory;

	Texture(std::shared_ptr<TextureFactory> f,uint32_t count)
	{
		image = nullptr;
		memory = nullptr;
		viewArray.clear();
		sampler = nullptr;

		property = TextureProperty{};

		factory = f;

		index = count;

		if (index == 85 || index == 86)
		{
			std::cout << "Stop" << std::endl;
		}
	}

	~Texture()
	{
		factory->addDefferedDestruct(image, memory, viewArray, sampler);

		std::cout << index << std::endl;
	}
};