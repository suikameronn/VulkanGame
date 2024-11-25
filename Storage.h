#pragma once
#include"Camera.h"
#include"Model.h"
#include<bitset>
#include<unordered_map>

#include"EnumList.h"

enum IMAGE
{
	IMAGETEST = 0,
};

class ImageData;

struct PrimitiveTextureCount
{
	uint32_t imageDataCount;
	VkPrimitiveTopology topology;

	/*
	bool operator==(PrimitiveTextureCount& another)
	{
		return (imageDataCount == another.imageDataCount) && (topology == another.topology);
	}

	bool operator!=(PrimitiveTextureCount& another)
	{
		return !((imageDataCount == another.imageDataCount) && (topology == another.topology));
	}
	*/
};

inline bool operator==(const PrimitiveTextureCount& lhs, const PrimitiveTextureCount& rhs) {
	return lhs.imageDataCount == rhs.imageDataCount && lhs.topology == rhs.topology;
}

struct Hash 
{
	size_t operator()(const DescriptorInfo info) const {
		size_t a = std::hash<VkDescriptorSetLayout>()(info.layout);
		size_t b = std::hash<VkPipeline>()(info.pipeline);
		size_t c = std::hash<VkPipelineLayout>()(info.pLayout);
		size_t d = std::hash<VkDescriptorPool>()(info.pool);

		return a ^ b ^ c ^ d;
	}

	size_t operator()(const PrimitiveTextureCount& ptc) const
	{
		size_t a = std::hash<uint32_t>()(ptc.imageDataCount);
		size_t b = std::hash<VkPrimitiveTopology>()(ptc.topology);

		return a ^ b;
	}
};

class Storage
{
private:
	std::unordered_map<OBJECT, std::shared_ptr<GltfModel>> fbxModelStorage;
	std::unordered_map<OBJECT, std::shared_ptr<Animation>> fbxAnimationStorage;
	std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDataStorage;

	std::unordered_map<PrimitiveTextureCount, DescriptorInfo,Hash> descriptorStorage;

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

	void addModel(OBJECT obj, GltfModel* geo);
	void addAnimation(OBJECT obj, Animation* animation);
	void addImageData(std::string, ImageData* image);
	void addDescriptorInfo(PrimitiveTextureCount ptc, DescriptorInfo& info);

	void setCamera(std::shared_ptr<Camera> c);
	void addModel(std::shared_ptr<Model> model);

	std::shared_ptr<GltfModel> getFbxModel(OBJECT obj);
	std::shared_ptr<Animation> getAnimation(OBJECT obj);
	std::shared_ptr<ImageData> getImageData(std::string path);

	DescriptorInfo* accessDescriptorInfo(PrimitiveTextureCount ptc);
	void accessDescriptorInfoItr(std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& begin,
		std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& end);

	std::shared_ptr<Camera> accessCamera();

	void accessFbxModel(std::unordered_map<OBJECT, std::shared_ptr<GltfModel>>::iterator& itr,
		std::unordered_map<OBJECT, std::shared_ptr<GltfModel>>::iterator& itr2);

	bool containModel(OBJECT obj);
	bool containAnimation(OBJECT obj);
	bool containImageData(std::string path);
	bool containDescriptorInfo(PrimitiveTextureCount ptc);

	static void FinishStorage()
	{
		delete storage;
	}
};