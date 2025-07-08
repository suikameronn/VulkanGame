#pragma once

#include"GpuPipelineLayoutBuilder.h"
#include"GpuDescriptorSetLayoutFactory.h"

#include"StructList.h"

enum class PipelineLayoutPattern
{
	PBR = 0,
	UI,
	CUBEMAP,
	CALCSHADOWMAP,
	CALCCUBEMAP,
	CALCIBL_DIFFUSE_SPECULAR,
	CALCIBL_BRDF
};

struct PipelineLayout;

struct PipelineLayoutHash
{
	size_t operator()(const std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>,std::vector<VkPushConstantRange>> a) const
	{
		size_t hash = a.first.size();

		hash ^= a.second.size() + 0x9e3779b9 + (hash << 6) + (hash >> 2);

		for (const auto& layout : a.first) {
			hash ^= layout->hashKey + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}

		for (const auto& pushConstant : a.second)
		{
			hash ^= pushConstant.offset + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= pushConstant.size + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= static_cast<uint32_t>(pushConstant.stageFlags) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}

		return hash;
	}
};

inline bool operator==(const std::shared_ptr<DescriptorSetLayout>& lhs, const std::shared_ptr<DescriptorSetLayout>& rhs) 
{
	// ������ nullptr �Ȃ瓙����
	if (!lhs && !rhs) return true;

	// �Е������� nullptr �Ȃ瓙�����Ȃ�
	if (!lhs || !rhs) return false;
	
	// �������L���Ȃ�A�w���I�u�W�F�N�g�̒��g���r����
	return *lhs == *rhs; // ��Œ�`���� DescriptorSetLayout �� operator== ���Ăяo��
}

inline bool operator==(const std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>, std::vector<VkPushConstantRange>> lhs
	, const std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>, std::vector<VkPushConstantRange>> rhs)
{
	for (int i = 0; i < lhs.first.size(); i++)
	{
		if (lhs.first[i] != rhs.first[i])
		{
			return false;
		}
	}

	for (int i = 0; i < lhs.second.size(); i++)
	{
		if (lhs.second[i].offset != rhs.second[i].offset ||
			lhs.second[i].size != rhs.second[i].size ||
			lhs.second[i].stageFlags != rhs.second[i].stageFlags)
		{
			return false;
		}
	}

	return true;
}

class GpuPipelineLayoutFactory : public std::enable_shared_from_this<GpuPipelineLayoutFactory>
{
private:

	//�_���f�o�C�X
	VkDevice device;

	//���݂̔j���\��̃��X�g�̃C���f�b�N�X
	//�`�揈�����s���邲�Ƃɒl���X�V�����
	uint32_t frameIndex;

	PipelineLayoutHash pipelineLayoutHash;

	//�r���_�[
	std::shared_ptr<GpuPipelineLayoutBuilder> builder;

	//���C�A�E�g���擾����
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;
	
	//���ɍ쐬�����p�C�v���C�����C�A�E�g���i�[����
	std::unordered_map<std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>, std::vector<VkPushConstantRange>>
		, std::weak_ptr<PipelineLayout>, PipelineLayoutHash> pipelineLayoutStorage;

	//�j���\��̃��\�[�X�̃��X�g
	std::array<std::list<VkPipelineLayout>, 2> destructList;

	//����̃��C�A�E�g����\���̂�ݒ肷��
	void convertLayouts(PipelineLayoutPattern pattern
		, std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts, std::vector<VkPushConstantRange>& pushConstant);

	//�r���_�[���g���ăp�C�v���C�����C�A�E�g���쐬����
	std::shared_ptr<PipelineLayout> createLayout(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts
		, std::vector<VkPushConstantRange>& pushConstants);

public:

	GpuPipelineLayoutFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutBuilder> b
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutF);

	~GpuPipelineLayoutFactory();

	//�p�C�v���C�����C�A�E�g�̍쐬
	std::shared_ptr<PipelineLayout> Create(PipelineLayoutPattern pattern, VkPipelineLayout& pLayout);

	//�p�C�v���C�����C�A�E�g�̍쐬
	std::shared_ptr<PipelineLayout> Create(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts
		, std::vector<VkPushConstantRange>& pushConstants, VkPipelineLayout& pLayout);

	//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
	void addDefferedDestruct(VkPipelineLayout& pLayout);

	//���\�[�X��j������
	void resourceDestruct();

};

struct PipelineLayout
{
	//�n�b�V���l
	size_t hashKey;

	VkPipelineLayout pLayout;

	//�p�C�v���C�����C�A�E�g�Ŏg�������C�A�E�g
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts;

	std::shared_ptr<GpuPipelineLayoutFactory> factory;

	PipelineLayout(std::shared_ptr<GpuPipelineLayoutFactory> f)
	{
		pLayout = nullptr;
		hashKey = 0;

		factory = f;
	}

	~PipelineLayout()
	{
		//�j���\�胊�X�g�Ƀ��\�[�X��������
		factory->addDefferedDestruct(pLayout);
	}
};