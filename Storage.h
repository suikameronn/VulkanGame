#pragma once
#include"Camera.h"
#include"Model.h"
#include<bitset>
#include<unordered_map>

enum OBJECT
{
	FBXTEST = 101,
	UNITYCHAN_NO_ANIM,
	GROUND1,
	FRAG
};

enum IMAGE
{
	IMAGETEST = 0,
};

class ImageData;

struct Hash {
	size_t operator()(const DescriptorInfo info) const {
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
	std::unordered_map<OBJECT, std::shared_ptr<FbxModel>> fbxModelStorage;
	std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDataStorage;

	std::unordered_map<uint32_t, DescriptorInfo> descriptorStorage;

	std::shared_ptr<Camera> camera;
	std::vector<std::shared_ptr<Model>> sceneModelStorage;

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

	std::vector<std::shared_ptr<Model>>::iterator sceneModelBegin() { return sceneModelStorage.begin(); }
	std::vector<std::shared_ptr<Model>>::iterator sceneModelEnd() { return sceneModelStorage.end(); }

	void addModel(OBJECT obj, FbxModel* geo);
	void addImageData(std::string, ImageData* image);
	void addDescriptorInfo(uint32_t imageDataCount, DescriptorInfo& info);

	void setCamera(std::shared_ptr<Camera> c);
	void addModel(std::shared_ptr<Model> model);

	std::shared_ptr<FbxModel> getFbxModel(OBJECT obj);
	std::shared_ptr<ImageData> getImageData(std::string path);

	DescriptorInfo* accessDescriptorInfo(uint32_t imageDataCount);
	void accessDescriptorInfoItr(std::unordered_map<uint32_t, DescriptorInfo>::iterator& begin,
		std::unordered_map<uint32_t, DescriptorInfo>::iterator& end);

	std::shared_ptr<Camera> accessCamera();

	void accessFbxModel(std::unordered_map<OBJECT, std::shared_ptr<FbxModel>>::iterator& itr,
		std::unordered_map<OBJECT, std::shared_ptr<FbxModel>>::iterator& itr2);

	bool containModel(OBJECT obj);
	bool containImageData(std::string path);
	bool containDescriptorInfo(uint32_t imageDataCount);

	static void FinishStorage()
	{
		delete storage;
	}
};