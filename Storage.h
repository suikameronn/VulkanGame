#pragma once
#include<bitset>
#include<unordered_map>

#include"EnumList.h"
#include"Light.h"
#include"Camera.h"
#include"Model.h"

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
		return a ^ b ^ c;
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
	std::unordered_map<GLTFOBJECT, std::shared_ptr<GltfModel>> gltfModelStorage;
	//std::unordered_map<OBJECT, std::shared_ptr<Animation>> gltfAnimationStorage;
	std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDataStorage;

	std::unordered_map<PrimitiveTextureCount, DescriptorInfo,Hash> descriptorStorage;

	std::shared_ptr<Camera> camera;
	std::vector<std::shared_ptr<Model>> sceneModelStorage;
	std::vector<std::shared_ptr<PointLight>> scenePointLightStorage;
	std::vector<std::shared_ptr<DirectionalLight>> sceneDirectionalLightStorage;

	VkDescriptorSetLayout lightLayout;
	VkDescriptorSet pointLightDescSet;
	VkDescriptorSet directionalLightDescSet;

	MappedBuffer pointLightsBuffer;
	MappedBuffer directionalLightsBuffer;

	Storage();
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

	std::vector<std::shared_ptr<Model>>& getModels() { return sceneModelStorage; }
	std::vector<std::shared_ptr<PointLight>>& getPointLights() { return scenePointLightStorage; }
	std::vector<std::shared_ptr<DirectionalLight>>& getDirectionalLights() { return sceneDirectionalLightStorage; }

	VkDescriptorSetLayout& getLightDescLayout() { return lightLayout; }
	VkDescriptorSet& getPointLightDescriptorSet() { return pointLightDescSet; }
	VkDescriptorSet& getDirectionalLightDescriptorSet() { return directionalLightDescSet; }

	void addModel(GLTFOBJECT obj, GltfModel* geo);
	//void addAnimation(OBJECT obj, Animation* animation);
	void addImageData(std::string, ImageData* image);
	void addDescriptorInfo(PrimitiveTextureCount ptc, DescriptorInfo& info);

	void setCamera(std::shared_ptr<Camera> c);
	void addModel(std::shared_ptr<Model> model);
	void addLight(std::shared_ptr<PointLight> light);
	void addLight(std::shared_ptr<DirectionalLight> light);

	void prepareLightsForVulkan();

	std::shared_ptr<GltfModel> getgltfModel(GLTFOBJECT obj);
	std::unordered_map<GLTFOBJECT, std::shared_ptr<GltfModel>>& getgltfModel();
	//std::shared_ptr<Animation> getAnimation(OBJECT obj);
	std::shared_ptr<ImageData> getImageData(std::string path);
	MappedBuffer& getPointLightsBuffer();
	MappedBuffer& getDirectionalLightsBuffer();

	DescriptorInfo* accessDescriptorInfo(PrimitiveTextureCount ptc);
	void accessDescriptorInfoItr(std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& begin,
		std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& end);

	std::shared_ptr<Camera> accessCamera();

	void accessgltfModel(std::unordered_map<GLTFOBJECT, std::shared_ptr<GltfModel>>::iterator& itr,
		std::unordered_map<GLTFOBJECT, std::shared_ptr<GltfModel>>::iterator& itr2);

	bool containModel(GLTFOBJECT obj);
	bool containAnimation(GLTFOBJECT obj);
	bool containImageData(std::string path);
	bool containDescriptorInfo(PrimitiveTextureCount ptc);

	void FinishStorage()
	{
		delete storage;
	}
};