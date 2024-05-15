#pragma once
#include"VulkanBase.h"
#include"VkModel.h"

#include<unordered_map>
#include<memory>
#include<bitset>

class VkStorage
{
private:
	//DescriptorSetLayoutのマップ
	std::unordered_map<std::bitset<8>,std::unique_ptr<VkDescriptorSetLayout>> layoutStorage;
	//DescriptorSetsのマップ
	std::unordered_map<std::pair<std::bitset<8>,ImageData*>,
		std::unique_ptr<std::vector<VkDescriptorSet>>> descriptorSetStorage;

public:
	//VkModelからuniformバッファやテクスチャの有無を判別して、DescriptorSetLayoutの参照それに与える
	void setDescriptorSetLayout(VkModel* model);
	//VkModelの持つDescriptorSetLayoutを参照して、VkModel用のDescriptorSetの参照を持たせる
	void setDescriptorSet(VkModel* model);
};