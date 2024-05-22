#pragma once
#include"Meshes.h"
#include"Material.h"
#include"FileManager.h"

#include<bitset>
#include<vulkan/vulkan.h>

enum OBJECT;

enum IMAGE;

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
};

class Storage
{
private:
	std::unordered_map<OBJECT, std::shared_ptr<Meshes>> meshesStorage;
	std::unordered_map<IMAGE, std::shared_ptr<ImageData>> imageStorage;

	std::unordered_map<std::bitset<8>, std::shared_ptr<DescriptorInfo>> descriptorStorage;

	Storage() {};
	static Storage* storage;
public:

	static Storage* GetInstance()
	{
		if (!storage)
		{
			storage = new Storage();
		}

		return storage;
	}

	void addObj(OBJECT obj, Meshes* geo);
	void addImage(IMAGE image, ImageData* imageData);
	void addDescriptorInfo(std::bitset<8> layoutBit, DescriptorInfo* info);

	Meshes* accessObj(OBJECT obj);
	ImageData* accessImage(IMAGE image);
	DescriptorInfo* accessDescriptorInfo(std::bitset<8> layoutBit);
	std::shared_ptr<Meshes> shareObj(OBJECT obj);
	std::shared_ptr<ImageData> shareImage(IMAGE image);
	std::shared_ptr<DescriptorInfo> shareDescriptor(std::bitset<8> layoutBit);

	bool containMeshes(OBJECT obj);
	bool containImageData(IMAGE image);
	bool containDescriptorInfo(std::bitset<8> layoutBit);

	uint32_t getSizeObjStorage();
	uint32_t getSizeImageStorage();
};