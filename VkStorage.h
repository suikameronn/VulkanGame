#pragma once
#include"VulkanBase.h"
#include"VkModel.h"

#include<unordered_map>
#include<memory>
#include<bitset>

class VkStorage
{
private:
	//DescriptorSetLayout�̃}�b�v
	std::unordered_map<std::bitset<8>,std::unique_ptr<VkDescriptorSetLayout>> layoutStorage;
	//DescriptorSets�̃}�b�v
	std::unordered_map<std::pair<std::bitset<8>,ImageData*>,
		std::unique_ptr<std::vector<VkDescriptorSet>>> descriptorSetStorage;

public:
	//VkModel����uniform�o�b�t�@��e�N�X�`���̗L���𔻕ʂ��āADescriptorSetLayout�̎Q�Ƃ���ɗ^����
	void setDescriptorSetLayout(VkModel* model);
	//VkModel�̎���DescriptorSetLayout���Q�Ƃ��āAVkModel�p��DescriptorSet�̎Q�Ƃ���������
	void setDescriptorSet(VkModel* model);
};