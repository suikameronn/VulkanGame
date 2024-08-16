#pragma once
#include<vector>
#include<bitset>
#include<vulkan/vulkan.h>
#include <time.h>

#include"Object.h"
#include"FbxModel.h"
#include"Animation.h"

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
	uint32_t imageDataCount;

	Rotate rotate;

	std::shared_ptr<FbxModel> fbxModel;

	std::vector<BufferObject> pointBuffers;
	std::vector<MappedBuffer> mappedBuffers;

	bool playAnim;

	clock_t startTime;
	clock_t currentTime;
	double deltaTime;

	std::string playAnimName = "walk";

public:

	Model();

	glm::vec3 scale;

	int hasAnimation() { return fbxModel->animationNum(); }
	void setFbxModel(std::shared_ptr<FbxModel> model);

	void startAnimation(std::string name);
	void playAnimation();
	bool hasPlayingAnimation() { return playAnim; }
	std::array<glm::mat4,251> getBoneInfoFinalTransform();

	uint32_t getTotalVertexNum() { return fbxModel->getTotalVertexNum(); }
	std::shared_ptr<Meshes> getMeshes(uint32_t i);
	uint32_t getMeshesSize();

	void setDescriptorSet(DescSetData* descSetData);

	BufferObject* getPointBuffer(uint32_t i);
	MappedBuffer* getMappedBuffer(uint32_t i);
	uint32_t getimageDataCount();
	DescSetData* getDescriptorSet();

	void updateTransformMatrix() override;
	void Update() override;

	void cleanupVulkan();
};