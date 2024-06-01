#pragma once
#include"Camera.h"
#include"Model.h"
#include<bitset>
#include<unordered_map>

enum OBJECT
{
	MODELTEST = 0,
	FBXTEST = 1000
};

enum IMAGE
{
	IMAGETEST = 0,
	BUCHDENOEL
};

class ImageData;

struct Hash {
	size_t operator()(const DescriptorInfo* info) const {
		size_t a = std::hash<VkDescriptorSetLayout>()(info->layout);
		size_t b = std::hash<VkPipeline>()(info->pipeline);
		size_t c = std::hash<VkPipelineLayout>()(info->pLayout);
		size_t d = std::hash<VkDescriptorPool>()(info->pool);

		return a ^ b ^ c ^ d;
	}
};

class Storage
{
private:
	std::unordered_map<OBJECT, std::unique_ptr<Meshes>> meshesStorage;
	std::unordered_map<IMAGE, std::unique_ptr<ImageData>> imageStorage;

	std::unordered_map<std::bitset<8>, DescriptorInfo*> descriptorStorage;

	Camera* camera;
	std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>,Hash> sceneModelStorage;

	Storage() {};
	~Storage()
	{
		cleanup();
	}
	static Storage* storage;

	void cleanup();
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

	void setCamera(Camera* c);
	void addModel(Model* model);

	Meshes* accessObj(OBJECT obj);
	ImageData* accessImage(IMAGE image);

	DescriptorInfo* accessDescriptorInfo(std::bitset<8> layoutBit);
	void accessDescriptorInfoItr(std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& begin,
		std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& end);

	Camera* accessCamera();

	void accessModelVector(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator current, 
		std::vector<std::unique_ptr<Model>>::iterator& itr, std::vector<std::unique_ptr<Model>>::iterator& itr2);
	void accessModelUnMap(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr,
		std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr2);

	bool containMeshes(OBJECT obj);
	bool containImageData(IMAGE image);
	bool containDescriptorInfo(std::bitset<8> layoutBit);

	uint32_t getSizeObjStorage();
	uint32_t getSizeImageStorage();

	static void FinishStorage()
	{
		delete storage;
	}
};