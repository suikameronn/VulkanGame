#pragma once
#include"Model.h"
#include<bitset>
#include<unordered_map>

enum OBJECT
{
	MODELTEST = 0
};

enum IMAGE
{
	IMAGETEST = 0
};

class ImageData;

struct Hash {
	size_t operator()(const DescriptorInfo& info) const {
		size_t a = std::hash<VkDescriptorSetLayout>()(info.layout);
		size_t b = std::hash<VkPipeline>()(info.pipeline);
		size_t c = std::hash<VkPipelineLayout>()(info.pLayout);
		size_t d = std::hash<VkDescriptorPool>()(info.pool);

		return a ^ b ^ c ^ d;
	}
};

class Storage
{
private:
	std::unordered_map<OBJECT, std::shared_ptr<Meshes>> meshesStorage;
	std::unordered_map<IMAGE, std::shared_ptr<ImageData>> imageStorage;

	std::unordered_map<std::bitset<8>, std::shared_ptr<DescriptorInfo>> descriptorStorage;

	std::unordered_map<DescriptorInfo, std::vector<std::unique_ptr<Model>>,Hash> sceneModelStorage;

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
	void addModel(Model* model);

	Meshes* accessObj(OBJECT obj);
	ImageData* accessImage(IMAGE image);
	DescriptorInfo* accessDescriptorInfo(std::bitset<8> layoutBit);
	void accessModelVector(std::unordered_map<DescriptorInfo, std::vector<std::unique_ptr<Model>>, Hash>::iterator current, 
		std::vector<std::unique_ptr<Model>>::iterator& itr, std::vector<std::unique_ptr<Model>>::iterator& itr2);
	void accessModelUnMap(std::unordered_map<DescriptorInfo, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr,
		std::unordered_map<DescriptorInfo, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr2);

	std::shared_ptr<Meshes> shareObj(OBJECT obj);
	std::shared_ptr<ImageData> shareImage(IMAGE image);
	std::shared_ptr<DescriptorInfo> shareDescriptor(std::bitset<8> layoutBit);

	bool containMeshes(OBJECT obj);
	bool containImageData(IMAGE image);
	bool containDescriptorInfo(std::bitset<8> layoutBit);

	uint32_t getSizeObjStorage();
	uint32_t getSizeImageStorage();
};