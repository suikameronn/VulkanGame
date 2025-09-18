#pragma once

#include"PipelineLayoutBuilder.h"
#include"DescriptorSetLayoutFactory.h"

#include"StructList.h"

enum class PipelineLayoutPattern
{
	PBR = 0,
	UI,
	CUBEMAP,
	CALC_SHADOWMAP,
	CALC_CUBEMAP,
	CALC_IBL_DIFFUSE,
	CALC_IBL_SPECULAR,
	CALC_IBL_BRDF,
	RAYCAST,
	COLIDER
};

struct PipelineLayout;

struct PipelineLayoutHash
{
	const uint64_t FNV_PRIME = 1099511628211LLU; // 2^40 + 2^8 + 0xB3 = 0x100000001b3
	const uint64_t FNV_OFFSET_BIAS = 14695981039346656037U; // 0xcbf29ce484222325

	void getHash(const uint32_t& u, size_t& hash) const
	{
		uint64_t u64 = u;

		hash ^= u64;
		hash *= FNV_PRIME;
	}

	void getHash(const float f, size_t& hash) const
	{
		uint32_t u;
		std::memcpy(&u, &f, sizeof(float));

		getHash(u, hash);
	}

	size_t operator()(const PipelineLayoutProperty& p) const
	{
		size_t hash = FNV_OFFSET_BIAS;

		getHash(static_cast<uint32_t>(p.layoutArray.size()), hash);
		for (int i = 0; i < p.layoutArray.size(); i++)
		{
			getHash(static_cast<uint32_t>(p.layoutArray[i]->hashKey), hash);
		}

		getHash(static_cast<uint32_t>(p.layoutArray.size()), hash);
		for (int i = 0; i < p.pushconstantArray.size(); i++)
		{
			getHash(static_cast<uint32_t>(p.pushconstantArray[i].size), hash);
			getHash(static_cast<uint32_t>(p.pushconstantArray[i].stageFlags), hash);
		}

		return hash;
	};
};

class PipelineLayoutFactory : public std::enable_shared_from_this<PipelineLayoutFactory>
{
private:

	//�_���f�o�C�X
	VkDevice device;

	//���݂̔j���\��̃��X�g�̃C���f�b�N�X
	//�`�揈�����s���邲�Ƃɒl���X�V�����
	uint32_t frameIndex;

	PipelineLayoutHash pipelineLayoutHash;

	//�r���_�[
	std::shared_ptr<PipelineLayoutBuilder> builder;

	//���C�A�E�g���擾����
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	
	//���ɍ쐬�����p�C�v���C�����C�A�E�g���i�[����
	std::unordered_map<PipelineLayoutProperty, std::weak_ptr<PipelineLayout>, PipelineLayoutHash> pipelineLayoutStorage;

	//�j���\��̃��\�[�X�̃��X�g
	std::array<std::list<VkPipelineLayout>, 2> destructList;

	//����̃��C�A�E�g����\���̂�ݒ肷��
	PipelineLayoutProperty convertLayouts(const PipelineLayoutPattern& pattern);

	//�r���_�[���g���ăp�C�v���C�����C�A�E�g���쐬����
	std::shared_ptr<PipelineLayout> createLayout(const PipelineLayoutProperty& property);

public:

	PipelineLayoutFactory(VkDevice& d, std::shared_ptr<PipelineLayoutBuilder> b
		, std::shared_ptr<DescriptorSetLayoutFactory> layoutF);

	~PipelineLayoutFactory();

	//�p�C�v���C�����C�A�E�g�̍쐬
	std::shared_ptr<PipelineLayout> Create(const PipelineLayoutPattern& pattern);

	//�p�C�v���C�����C�A�E�g�̍쐬
	std::shared_ptr<PipelineLayout> Create(const PipelineLayoutProperty& property);

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

	//VkPushconstant�̔z��
	std::vector<VkPushConstantRange> pushconstants;

	std::shared_ptr<PipelineLayoutFactory> factory;

	PipelineLayout(std::shared_ptr<PipelineLayoutFactory> f)
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