#pragma once
#include<vector>

#include"VulkanBase.h"
#include<glm/glm.hpp>
#include<bitset>

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	/*
	alignas(16) glm::mat3 normal;
	alignas(16) glm::vec3 diffuse;
	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 specular;
	alignas(16) glm::vec3 emissive;
	alignas(16) glm::vec3 transmissive;
	float shininess;
	*/
};

class VkObject
{
protected:
	UniformBufferObject ubo;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMappedPoint;

	std::bitset<8> layoutBit;//右から、テクスチャ...とフラグを立てていく、
	VkDescriptorSetLayout* layout;
	VkDescriptorPool* pool;
	VkDescriptorSet* descriptorSet;

public:
	std::bitset<8> getLayoutBit();
	void setDescriptorSetLayout(VkDescriptorSetLayout* layout);
	void setDescriptorPool(VkDescriptorPool* pool);
	void setDescriptorSet(VkDescriptorSet* descriptorSet);

	VkDescriptorSetLayout* getDescriptorSetLayout();

	VkBuffer* getUniformBuffer();
};