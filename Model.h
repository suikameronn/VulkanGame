#pragma once
#include<vector>
#include<bitset>
#include<vulkan/vulkan.h>

#include"Object.h"
#include"FbxModel.h"

template<typename T>
uint32_t getSize(T v)
{
	return v.size();
}

struct BufferObject
{
	VkBuffer vertBuffer;
	VkDeviceMemory vertHandler;

	VkBuffer indeBuffer;
	VkDeviceMemory indeHandler;
};

struct DescriptorInfo
{
	VkDescriptorSetLayout layout;
	VkDescriptorPool pool;
	VkPipelineLayout pLayout;
	VkPipeline pipeline;

	bool operator==(const DescriptorInfo& a) const
	{
		return layout == a.layout;
	}

	bool operator!=(const DescriptorInfo& a) const
	{
		return !(layout == a.layout);
	}
};

struct MappedBuffer
{
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;
};

class Model:public Object
{
protected:
	std::bitset<8> layoutBit;

	std::shared_ptr<FbxModel> fbxModel;

	std::vector<BufferObject> pointBuffers;
	std::vector<MappedBuffer> mappedBuffers;

	std::vector<DescriptorInfo*> descriptorInfos;
	std::vector<VkDescriptorSet> descriptorSets;//VkDescriptorPoolÇ™îjä¸Ç≥ÇÍÇÍÇŒé©ìÆÇ≈è¡Ç≥ÇÍÇÈÅB

public:

	Model();
	~Model();

	void setFbxModel(std::shared_ptr<FbxModel> model);

	std::shared_ptr<Meshes> getMeshes(uint32_t i);
	uint32_t getMeshesSize();

	//Material* getMaterial();

	void setDescriptorInfo(DescriptorInfo* info);
	void setDescriptorSet(VkDescriptorSet* descriptorSet);

	DescriptorInfo* getDescriptorInfo(uint32_t i);

	BufferObject* getPointBuffer(uint32_t i);
	MappedBuffer* getMappedBuffer(uint32_t i);
	std::bitset<8> getLayoutBit();
	VkDescriptorSet* getDescriptorSet(uint32_t i);

	void Update() override;
};