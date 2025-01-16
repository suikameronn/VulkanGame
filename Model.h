#pragma once
#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"

class Scene;

template<typename T>
uint32_t getSize(T v)
{
	return v.size();
}

class Model:public Object
{
protected:
	Scene* scene;

	uint32_t imageDataCount;

	std::unique_ptr<PhysicBase> physicBase;

	std::shared_ptr<GltfModel> gltfModel;

	std::vector<BufferObject> pointBuffers;
	std::vector<MappedBuffer> mappedBuffers;
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	clock_t startTime;
	clock_t currentTime;
	double deltaTime;

	std::shared_ptr<Material> material;
	std::shared_ptr<Colider> colider;

	std::string defaultAnimationName;
	std::string currentPlayAnimationName;
	std::vector<std::string> animationNames;

public:

	Model();
	Model(std::string luaScriptPath);
	~Model() {};

	float gravity;
	float slippery;

	glm::vec3 scale;

	void setDefaultAnimationName(std::string name);

	void bindObject(std::weak_ptr<Object> obj);
	void bindCamera(std::weak_ptr<Camera> camera);

	void sendPosToChildren(glm::vec3 pos);

	void setgltfModel(std::shared_ptr<GltfModel> model);
	GltfNode* getRootNode() { return gltfModel->getRootNode(); }
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	void setMaterial(std::shared_ptr<Material> material) { this->material = material; }
	void setDiffuse(glm::vec4 color) { this->material->setDiffuse(color); }

	void switchPlayAnimation();
	void switchPlayAnimation(std::string nextAnimation);
	void playAnimation();
	std::array<glm::mat4, 128>& getJointMatrices(int index);

	std::vector<DescSetData> descSetDatas;
	BufferObject* getPointBufferData() { return pointBuffers.data(); }
	MappedBuffer* getMappedBufferData() { return mappedBuffers.data(); };
	uint32_t getimageDataCount();

	bool isMovable;
	bool hasColider();
	void setColider();
	std::shared_ptr<Colider> getColider() { return colider; }

	std::shared_ptr<Model> rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength);

	void updateTransformMatrix() override;

	void cleanupVulkan();

	void setPosition(glm::vec3 pos) override;

	void Update() override;
	void customUpdate() override;

	void initFrameSetting() override;

	bool isGround();
};