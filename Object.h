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

#include <lua.hpp>
#include <lualib.h>
#include <lauxlib.h>

#include"EnumList.h"
#include"Controller.h"
#include"Camera.h"
#include"PhysicBase.h"

struct BufferObject
{
	VkBuffer vertBuffer;
	VkDeviceMemory vertHandler;

	VkBuffer indeBuffer;
	VkDeviceMemory indeHandler;
};

struct DescriptorInfo
{
	VkPipelineLayout pLayout;
	VkPipeline pipeline;
};

struct MappedBuffer
{
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;

	void destroy(VkDevice& device)
	{
		vkDestroyBuffer(device, uniformBuffer, nullptr);
		vkFreeMemory(device, uniformBufferMemory, nullptr);
		uniformBufferMapped = nullptr;
	}
};

struct DescSetData
{
	VkPrimitiveTopology topology;
	VkDescriptorSet descriptorSet;
};

struct Rotate
{
	float x;
	float y;
	float z;

	float getRadian(float deg)
	{
		return static_cast<float>(deg * (PI / 180.0f));
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

	lua_State* lua;
	std::string luaPath;

	ObjNum objNum;
	std::vector<Tag> tags;

	std::unique_ptr<PhysicBase> physicBase;

	std::weak_ptr<Camera> cameraObj;
	std::vector<Object*> childObjects;

	float rotateSpeed;
	float length;

	glm::vec3 parentPos;
	glm::vec3 position;

	virtual glm::vec3 inputMove();

public:
	Object();

	ObjNum getObjNum() { return objNum; }
	bool containTag(Tag tag);

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

	void bindObject(Object* obj);
	void bindCamera(std::weak_ptr<Camera> camera);
	void sendPosToChildren(glm::vec3 pos);
	void setParentPos(glm::vec3 parentPos);

	virtual void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	glm::mat4 getTransformMatrix();

	glm::mat4 getRodriguesMatrix(glm::vec3 axis, float theta);

	virtual void initFrameSetting();
	virtual void registerGlueFunctions();
	virtual void updateTransformMatrix() {};
	virtual void Update();
	virtual void customUpdate() {}
};