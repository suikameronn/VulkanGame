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
	lua_State* coroutine;
	std::string luaPath;

	ObjNum objNum;
	std::vector<Tag> tags;

	std::unique_ptr<PhysicBase> physicBase;

	std::weak_ptr<Camera> cameraObj;
	std::vector<Object*> childObjects;

	float rotateSpeed;
	float length;

	glm::vec3 lastPos;
	Rotate lastRotate;
	glm::vec3 lastScale;

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
	void sendPosToChildren();
	void setParentPos(glm::vec3 lastPos, glm::vec3 currentPos);

	virtual void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	glm::vec3 getLastPosition();
	Rotate getLastRotate();
	glm::vec3 getLastScale();
	virtual void setLastFrameTransform();//一つ前のフレームの座標などのデータを設定

	glm::mat4 getTransformMatrix();

	virtual void createTransformTable();
	virtual void initFrameSetting();
	virtual void registerGlueFunctions();
	virtual void updateTransformMatrix() {};
	virtual void Update();
	virtual void customUpdate() {};
	virtual void sendTransformToLua();
	virtual void receiveTransformFromLua();
};

namespace glueObjectFunction//Objectクラス用のglue関数
{
	static int glueSetPos(lua_State* lua)
	{
		//lua_getglobal(lua, "object");
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		{
			float x = static_cast<float>(lua_tonumber(lua, -4));
			float y = static_cast<float>(lua_tonumber(lua, -3));
			float z = static_cast<float>(lua_tonumber(lua, -2));
			obj->setPosition(glm::vec3(x, y, z));
			break;
		}
		}

		return 0;
	}

	static int glueSetRotate(lua_State* lua)
	{
		//lua_getglobal(lua, "object");
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
			obj->rotate.x = static_cast<float>(lua_tonumber(lua, -4));
			obj->rotate.y = static_cast<float>(lua_tonumber(lua, -3));
			obj->rotate.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}