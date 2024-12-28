#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define PI 3.14159265359

#include<iostream>
#include<vector>
#include<math.h>
#include<memory>

#include<vulkan/vulkan.h>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"EnumList.h"
#include"Controller.h"
#include"Camera.h"

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
	VkPrimitiveTopology topology;
	VkDescriptorSet descriptorSet;
};

//アニメーションの遷移などを担う
class UpdateScript
{
	std::string currentAnimationName;

	void initScript(std::string path);

public:
	UpdateScript(std::string path);
	~UpdateScript();

	void update();
	std::string getCurrentAnimationName() { return currentAnimationName; }

};

struct Rotate
{
	float x;
	float y;
	float z;

	float getRadian(float deg)
	{
		return deg * (PI / 180.0f);
	}

	glm::mat4 getRotateMatrix()
	{
		return glm::rotate(glm::mat4(1.0f), getRadian(z), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(y), glm::vec3(0.0f, -1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(x), glm::vec3(1.0f, 0.0f, 0.0f));
	}
};

class Object
{
protected:
	ObjNum objNum;

	std::unique_ptr<UpdateScript> updateScript;
	std::weak_ptr<Camera> cameraObj;
	std::vector<std::weak_ptr<Object>> childObjects;

	float rotateSpeed;
	float length;

	glm::vec3 parentPos;
	glm::vec3 position;

	virtual glm::vec3 inputMove();

public:
	Object();

	ObjNum getObjNum() { return objNum; }
	void setLuaScript(std::string path);

	bool spherePos;

	bool uniformBufferChange;

	Rotate rotate;

	float posOffSet;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	glm::vec3 axis;

	glm::mat4 transformMatrix;

	void bindObject(std::weak_ptr<Object> obj);
	void bindCamera(std::weak_ptr<Camera> camera);
	void sendPosToChildren(glm::vec3 pos);
	void setParentPos(glm::vec3 parentPos);

	virtual void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	glm::mat4 getTransformMatrix();

	virtual void initFrameSetting();
	virtual void updateTransformMatrix() {};
	virtual void Update();
	virtual void customUpdate() {}
};