#pragma once
#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"Object.h"
#include"FbxModel.h"
#include"Animation.h"
#include"Colider.h"

template<typename T>
uint32_t getSize(T v)
{
	return v.size();
}

struct Rotate
{
	float x;
	float y;
	float z;

	float getRadian(float deg)
	{
		return deg * (PI / 180.0f);
	}

	glm::mat4 getRotateMatrix()
	{
		return glm::rotate(glm::mat4(1.0f), getRadian(z), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(y), glm::vec3(0.0f, -1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(x), glm::vec3(1.0f, 0.0f, 0.0f));
	}
};

class Model:public Object
{
protected:
	uint32_t imageDataCount;

	std::shared_ptr<FbxModel> fbxModel;

	std::vector<BufferObject> pointBuffers;
	std::vector<MappedBuffer> mappedBuffers;

	bool playAnim;

	clock_t startTime;
	clock_t currentTime;
	double deltaTime;

	ACTION action;
	ACTION defaultAction;

	glm::vec3 inputMove() override;

	std::shared_ptr<Colider> colider;

	void changeAction();
	void changeAction(ACTION act);

public:

	Model();

	Rotate rotate;
	glm::vec3 scale;

	int hasAnimation() { return fbxModel->animationNum(); }
	void setFbxModel(std::shared_ptr<FbxModel> model);
	void setAnimation(std::shared_ptr<FbxModel> model, std::string fileName, ACTION action);

	void setDefaultAction(ACTION act) { defaultAction = act; }
	void startAnimation();
	void playAnimation();
	bool hasPlayingAnimation() { return playAnim; }
	std::array<glm::mat4,250> getBoneInfoFinalTransform();

	uint32_t getTotalVertexNum() { return fbxModel->getTotalVertexNum(); }
	std::shared_ptr<Meshes> getMeshes(uint32_t i);
	uint32_t getMeshesSize();

	void setDescriptorSet(DescSetData* descSetData);

	BufferObject* getPointBuffer(uint32_t i);
	MappedBuffer* getMappedBuffer(uint32_t i);
	uint32_t getimageDataCount();
	DescSetData* getDescriptorSet();

	void setColider(COLIDER shape,float right,float left,float top,float bottom, float front, float back);
	void setColider(COLIDER shape);
	bool hasColider();
	std::shared_ptr<Colider> getColider() { return colider; }

	void updateTransformMatrix() override;
	void Update() override;

	void cleanupVulkan();
};