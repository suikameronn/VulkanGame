#pragma once

#include<iostream>
#include<vector>
#include<memory>
#include<unordered_map>

#include"DescriptorSetLayoutBuilder.h"

#define MAX_TEXTURE_COUNT 5

struct DescriptorSetLayout;
struct LayoutHash;

struct LayoutHash
{
	size_t genHash(const VkDescriptorSetLayoutBinding& a) const
	{
		size_t h1 = std::hash<uint32_t>{}(a.binding);
		size_t h2 = std::hash<VkDescriptorType>{}(a.descriptorType);
		size_t h3 = std::hash<uint32_t>{}(a.descriptorCount);
		size_t h4 = std::hash<VkShaderStageFlags>{}(a.stageFlags);
		size_t h5 = std::hash<const VkSampler*>{}(a.pImmutableSamplers);

		size_t seed = 0;

		seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h5 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

		return seed;
	}

	size_t genHash(const std::vector<VkDescriptorSetLayoutBinding>& a) const
	{
		size_t hash = a.size();

		for (const auto& binding : a) {
			hash ^= genHash(binding) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}
		return hash;
	}

	size_t operator()(const std::vector<VkDescriptorSetLayoutBinding>& a) const
	{
		size_t hash = a.size();

		for (const auto& binding : a) {
			hash ^= genHash(binding) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}
		return hash;
	}
};

inline bool operator==(const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs) {
	return lhs.binding == rhs.binding &&
		lhs.descriptorType == rhs.descriptorType &&
		lhs.descriptorCount == rhs.descriptorCount &&
		lhs.stageFlags == rhs.stageFlags &&
		lhs.pImmutableSamplers == rhs.pImmutableSamplers;
}

enum class LayoutPattern
{
	MODELANIMMAT = 0,//���f���s��ƃA�j���[�V�����s��
	CAMERA,//�J�����̍��W�A�r���[�s��ƃv���W�F�N�V�����s��
	MATERIAL,//�e��e�N�X�`�����܂�
	LIGHT,//���C�g�̐F
	SHADOWMAP,//�V���h�E�}�b�v�̃��C�g���_�̍s��ƃf�v�X�o�b�t�@�̃e�N�X�`��
	IBL,//IBL�̃f�B�t�[�Y�A�X�y�L�����[�ABRDF���܂�
	RAYCAST,//��̃X�g���[�W�o�b�t�@
	UI,//UI�p�̕ϊ��s��ƃe�N�X�`��
	SINGLE_TEX_FLAG,//�P��̃e�N�X�`��(�t���O�����g�V�F�[�_�p)
	SINGLE_UNIFORM_VERT,//�P��̃��j�t�H�[���o�b�t�@(���_�V�F�[�_�p)
	SINGLE_UNIFORM_FRAG//�P��̃��j�t�H�[���o�b�t�@(�t���O�����g�V�F�[�_�p)
};

class DescriptorSetLayoutFactory : public std::enable_shared_from_this<DescriptorSetLayoutFactory>
{
private:

	LayoutHash layoutHash;

	//�o�C���f�B���O���L�[�ɁA����g�ݍ��킹�̃��C�A�E�g����̂ݕۑ�����
	std::unordered_map<std::vector<VkDescriptorSetLayoutBinding>,
		std::weak_ptr<DescriptorSetLayout>,LayoutHash> layoutStorage;

	//�_���f�o�C�X
	VkDevice device;

	//���݂̔j���\��̃��X�g�̃C���f�b�N�X
	//�`�揈�����s���邲�Ƃɒl���X�V�����
	uint32_t frameIndex;

	//�r���_�[
	std::shared_ptr<DescriptorSetLayoutBuilder> builder;

	//�j���\��̃��\�[�X�̃��X�g
	std::array<std::list<VkDescriptorSetLayout>, 2> destructList;

	//���C�A�E�g�̍쐬
	std::shared_ptr<DescriptorSetLayout> createLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	//����̃��C�A�E�g����r���f�B���O��Ԃ�
	void convertBinding(const LayoutPattern& pattern, std::vector<VkDescriptorSetLayoutBinding>& bindings);

public:

	DescriptorSetLayoutFactory(VkDevice& d, std::shared_ptr<DescriptorSetLayoutBuilder> b);

	~DescriptorSetLayoutFactory();

	std::shared_ptr<DescriptorSetLayoutBuilder> getBuilder()
	{
		return builder;
	}

	//���C�A�E�g�̍쐬
	std::shared_ptr<DescriptorSetLayout> Create(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	
	//����̃v���p�e�B����I��
	std::shared_ptr<DescriptorSetLayout> Create(const LayoutPattern& pattern);

	//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
	void addDefferedDestruct(VkDescriptorSetLayout& layout);

	//���\�[�X��j������
	void resourceDestruct();
};

struct DescriptorSetLayout
{
	VkDescriptorSetLayout layout;
	std::shared_ptr<DescriptorSetLayoutFactory> factory;

	//unordered_map�ɓo�^����Ƃ��Ɏg���n�b�V���l
	size_t hashKey;

	DescriptorSetLayout(std::shared_ptr<DescriptorSetLayoutFactory> f)
	{
		layout = nullptr;
		factory = f;
	}

	~DescriptorSetLayout()
	{
		//�j���\�胊�X�g��layout��ǉ�����
		factory->addDefferedDestruct(layout);
	}

	bool operator==(const DescriptorSetLayout& other) const
	{
		if (hashKey == other.hashKey)
		{
			return true;
		}

		return false;
	}
};