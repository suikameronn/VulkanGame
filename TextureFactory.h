#pragma once

#include"TextureBuilder.h"

enum class TexturePattern
{
	NORMAL,
	CUBEMAP,
	CALC_CUBEMAP
};

class TextureFactory : public std::enable_shared_from_this<TextureFactory>
{
private:

	//�t���[���C���f�b�N�X
	uint32_t frameIndex;

	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	//�r���_�[
	std::shared_ptr<TextureBuilder> builder;

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

public:

	TextureFactory(std::shared_ptr<VulkanCore> core, std::shared_ptr<TextureBuilder> b);

	//�r���_�[���擾
	std::shared_ptr<TextureBuilder> getBuilder()
	{
		return builder;
	}

	//�v���Z�b�g�̃v���p�e�B���擾
	TextureProperty getPreset(const TexturePattern& pattern);

	//�O���̉摜�t�@�C������e�N�X�`�������ꍇ�𕪂���
	std::shared_ptr<Texture> Create(const std::string& filePath, TextureProperty& property);

	//�O���̉摜�t�@�C������e�N�X�`�������ꍇ�𕪂���
	//����̃v���p�e�B���g��
	std::shared_ptr<Texture> Create(const std::string& filePath, const uint32_t& width
		, const uint32_t& height, const TexturePattern& pattern);

	//�摜�f�[�^��z���Ŏ󂯎��
	std::shared_ptr<Texture> Create(uint32_t& texChannel, unsigned char* pixels
		, const uint32_t& width, const uint32_t& height, const TexturePattern& pattern);

	//�摜�f�[�^�͓��ꂸ�ɁA�e�N�X�`���o�b�t�@�݂̂����
	std::shared_ptr<Texture> Create(TextureProperty& property);
	std::shared_ptr<Texture> Create(const uint32_t& width, const uint32_t& height
		, const TexturePattern& pattern);

	//�X���b�v�`�F�[���p
	std::shared_ptr<Texture> ImageViewCreate(TextureProperty& property);
	std::shared_ptr<Texture> ViewCreate(TextureProperty& property, VkImage& image);

	//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
	void addDefferedDestruct(VkImage& image, VkDeviceMemory& memory
		, VkImageView& view, VkSampler& sampler);

	//���\�[�X��j������
	void resourceDestruct();
};

struct Texture
{
	//�e�N�X�`���̉摜�f�[�^
	VkImage image;
	VkDeviceMemory memory;
	//�摜�̃r���[
	VkImageView view;
	//�摜�̃T���v���[ �e�N�X�`���̋��E�̐ݒ�Ȃ�
	VkSampler sampler;

	//�e��v���p�e�B���L�^
	TextureProperty property;

	std::shared_ptr<TextureFactory> factory;

	Texture(std::shared_ptr<TextureFactory> f)
	{
		image = nullptr;
		memory = nullptr;
		view = nullptr;
		sampler = nullptr;

		property = TextureProperty{};

		factory = f;
	}

	~Texture()
	{
		factory->addDefferedDestruct(image, memory, view, sampler);
	}
};