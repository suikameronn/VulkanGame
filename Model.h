#pragma once
#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"

template<typename T>
uint32_t getSize(T v)
{
	return v.size();
}

class Model:public Object
{
protected:
	uint32_t imageDataCount;

	std::shared_ptr<GltfModel> gltfModel;

	std::vector<BufferObject> pointBuffers;
	std::vector<MappedBuffer> mappedBuffers;
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	bool playAnim;
	bool loopAnimation;

	clock_t startTime;
	clock_t currentTime;
	double deltaTime;

	std::shared_ptr<Material> material;
	std::shared_ptr<Colider> colider;

public:

	Model();
	Model(std::string luaScriptPath);

	glm::vec3 scale;

	void bindObject(std::weak_ptr<Object> obj);
	void bindCamera(std::weak_ptr<Object> camera);

	void sendPosToChildren(glm::vec3 pos);

	void setgltfModel(std::shared_ptr<GltfModel> model);
	GltfNode* getRootNode() { return gltfModel->getRootNode(); }
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	void setMaterial(std::shared_ptr<Material> material) { this->material = material; }

	void playAnimation();
	std::array<glm::mat4, 128>& getJointMatrices(int index);

	std::vector<DescSetData> descSetDatas;
	BufferObject* getPointBuffer(uint32_t i);
	MappedBuffer* getMappedBuffer(uint32_t i);
	uint32_t getimageDataCount();

	bool hasColider();
	void setColider();
	std::shared_ptr<Colider> getColider() { return colider; }

	void updateTransformMatrix() override;

	void cleanupVulkan();

	void setPosition(glm::vec3 pos) override;

	void customUpdate() {}
};