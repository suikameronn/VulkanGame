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
	std::unordered_map<OBJECT, std::shared_ptr<FbxModel>> fbxModelStorage;

	std::unordered_map<std::bitset<8>, DescriptorInfo*> descriptorStorage;

	Camera* camera;
	std::unordered_map<DescriptorInfo*, std::vector<std::shared_ptr<Model>>,Hash> sceneModelStorage;

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

	void addModel(OBJECT obj, FbxModel* geo);
	void addDescriptorInfo(std::bitset<8> layoutBit, DescriptorInfo* info);

	void setCamera(Camera* c);
	void addModel(Model* model);

	std::shared_ptr<FbxModel> getFbxModel(OBJECT obj);

	DescriptorInfo* accessDescriptorInfo(std::bitset<8> layoutBit);
	void accessDescriptorInfoItr(std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& begin,
		std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& end);

	Camera* accessCamera();

	void accessModelVector(std::unordered_map<DescriptorInfo*, std::vector<std::shared_ptr<Model>>, Hash>::iterator current,
		std::vector<std::shared_ptr<Model>>::iterator& itr, std::vector<std::shared_ptr<Model>>::iterator& itr2);
	void accessModelUnMap(std::unordered_map<DescriptorInfo*, std::vector<std::shared_ptr<Model>>, Hash>::iterator* itr,
		std::unordered_map<DescriptorInfo*, std::vector<std::shared_ptr<Model>>, Hash>::iterator* itr2);

	bool containModel(OBJECT obj);
	bool containDescriptorInfo(std::bitset<8> layoutBit);

	static void FinishStorage()
	{
		delete storage;
	}
};