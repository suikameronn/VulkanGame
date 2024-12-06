#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define PI 3.14159265359

#include<iostream>
#include<vector>
#include<math.h>
#include<vulkan/vulkan.h>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"EnumList.h"
#include"Controller.h"

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

	void destroy(VkDevice& device)
	{
		vkDestroyDescriptorPool(device, pool, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
	}
};

struct MappedBuffer
{
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;
};

struct DescSetData
{
	uint32_t texCount;
	VkPrimitiveTopology topology;
	VkDescriptorSet decriptorSet;
};

class Object
{
protected:
	std::weak_ptr<Object> cameraObj;
	std::vector<std::weak_ptr<Object>> childObjects;
	float theta, phi;

	float rotateSpeed;
	float length;

	glm::vec3 parentPos;
	glm::vec3 position;

	virtual glm::vec3 inputMove();

public:

	Object();

	bool controllable;
	float speed;

	bool spherePos;

	bool uniformBufferChange;

	float posOffSet;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	glm::vec3 axis;

	glm::mat4 transformMatrix;

	void bindObject(std::weak_ptr<Object> obj);
	void bindCamera(std::weak_ptr<Object> camera);
	void sendPosToChildren(glm::vec3 pos);
	void setParentPos(glm::vec3 parentPos);

	virtual void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	void setSpherePos(float theta, float phi);

	glm::mat4 getTransformMatrix();

	virtual void updateTransformMatrix() {};
	virtual void Update();
};