#pragma once

#include"DescriptorSetLayoutFactory.h"

struct PipelineLayoutProperty
{
	//�p�C�v���C���Ɏg�����C�A�E�g�̃v���p�e�B
	std::vector<std::shared_ptr<DescriptorSetLayout>> layoutArray;

	//�v�b�V���R���X�^���g
	std::vector<VkPushConstantRange> pushconstantArray;

	void initProperty()
	{
		layoutArray.clear();

		pushconstantArray.clear();
	}

	inline bool operator==(const PipelineLayoutProperty& other) const
	{
		if (layoutArray.size() != other.layoutArray.size())
		{
			return false;
		}

		for (int i = 0; i < layoutArray.size(); i++)
		{
			if (layoutArray[i]->hashKey != other.layoutArray[i]->hashKey
				 || layoutArray[i]->layout != other.layoutArray[i]->layout)
			{
				return false;
			}
		}

		if (pushconstantArray.size() != other.pushconstantArray.size())
		{
			return false;
		}

		for (int i = 0; i < pushconstantArray.size(); i++)
		{
			if (pushconstantArray[i].size != other.pushconstantArray[i].size
				|| pushconstantArray[i].stageFlags != other.pushconstantArray[i].stageFlags)
			{
				return false;
			}
		}

		return true;
	}
};

class PipelineLayoutBuilder : public std::enable_shared_from_this<PipelineLayoutBuilder>
{
private:

	PipelineLayoutProperty property;

	//VkDescriptorSetLayout�̃t�@�N�g��
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;

public:

	PipelineLayoutBuilder();

	~PipelineLayoutBuilder()
	{
#ifdef _DEBUG
		std::cout << "PipelineLayoutBuilder :: �f�X�g���N�^" << std::endl;
#endif
	}

	//���C�A�E�g������������
	std::shared_ptr<PipelineLayoutBuilder> initProperty();

	//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
	std::shared_ptr<PipelineLayoutBuilder> addLayout(const std::shared_ptr<DescriptorSetLayout> layout);

	//VkPushConstant��������
	std::shared_ptr<PipelineLayoutBuilder> addPushConstant(const uint32_t& size, const VkShaderStageFlags& stage);

	//�p�C�v���C�����C�A�E�g���쐬����
	PipelineLayoutProperty Build();
};