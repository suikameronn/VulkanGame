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
	//GraphicsPipeline�̃}�b�v
	std::unordered_map<std::bitset<8>, std::unique_ptr<VkPipeline>> pipelineStorage;
	//DescriptorPool�̃}�b�v
	std::unordered_map<std::bitset<8>, std::unique_ptr<VkDescriptorPool>> poolStorage;
	//DescriptorSets�̃}�b�v
	std::unordered_map<std::pair<std::bitset<8>,ImageData*>,
		std::unique_ptr<VkDescriptorSet>> descriptorSetStorage;

public:
	//VkModel����uniform�o�b�t�@��e�N�X�`���̗L���𔻕ʂ��āADescriptorSetLayout�̎Q�Ƃ���ɗ^����
	void setDescriptorSetLayout(VkModel* model);
	//VkModel�Ƀp�C�v���C���̎Q�Ƃ�^����
	void setPipeline(VkModel* model);
	//VkDescriptorPool����������
	void setDescriptorPool(VkModel* model);
	//VkModel�̎���DescriptorSetLayout���Q�Ƃ��āAVkModel�p��DescriptorSet�̎Q�Ƃ���������
	void setDescriptorSet(VkModel* model);
};