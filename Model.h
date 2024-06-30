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

struct Rotate
{
	float radian;
	glm::vec3 direction;
};

class Model:public Object
{
protected:
	std::bitset<8> layoutBit;

	glm::mat4 transformMatrix;
	Rotate rotate;

	std::shared_ptr<FbxModel> fbxModel;

	std::vector<BufferObject> pointBuffers;
	std::vector<MappedBuffer> mappedBuffers;

	std::vector<DescriptorInfo*> descriptorInfos;
	std::vector<VkDescriptorSet> descriptorSets;//VkDescriptorPoolÇ™îjä¸Ç≥ÇÍÇÍÇŒé©ìÆÇ≈è¡Ç≥ÇÍÇÈÅB

public:

	Model();

	glm::vec3 scale;

	void setFbxModel(std::shared_ptr<FbxModel> model);

	std::shared_ptr<Meshes> getMeshes(uint32_t i);
	uint32_t getMeshesSize();

	void setDescriptorInfo(DescriptorInfo* info);
	void setDescriptorSet(VkDescriptorSet* descriptorSet);

	DescriptorInfo* getDescriptorInfo(uint32_t i);

	BufferObject* getPointBuffer(uint32_t i);
	MappedBuffer* getMappedBuffer(uint32_t i);
	std::bitset<8> getLayoutBit();
	VkDescriptorSet* getDescriptorSet(uint32_t i);

	glm::mat4 getTransformMatrix();

	glm::vec3 getAverageLocalPos();

	void updateTransformMatrix() override;
	void Update() override;

	void cleanupVulkan();
};