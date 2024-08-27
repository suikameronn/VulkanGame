#pragma once

#include"assimp/Importer.hpp"      // C++ importer interface
#include"assimp/scene.h"          // Output data structure
#include"assimp/postprocess.h"     // Post processing flags

#include"Meshes.h"
#include"Material.h"
#include"Animation.h"

class FbxModel
{
private:

	uint32_t totalVertNum;
	std::vector<std::shared_ptr<Meshes>> meshes;

	float startFrame;
	float endFrame;

	bool loopAnim;
	bool finish;

	glm::vec3 averageLocalPos;

	std::unordered_map<std::string, std::shared_ptr<Pose>> poses;
	std::unordered_map<std::string,std::shared_ptr<Animation>> animations;

	uint32_t imageDataCount;

	glm::mat4 globalInverseTransform;
	BoneInfo boneInfo;
	std::unordered_map<std::string, int> m_BoneNameToIndexMap;

public:

	FbxModel();

	void addMeshes(Meshes* mesh);

	std::shared_ptr<Meshes> getMeshes(uint32_t i);

	void cleanupVulkan();

	void setTotalVertexNum(uint32_t total) { totalVertNum = total; }
	uint32_t getTotalVertexNum() { return totalVertNum; }

	uint32_t getMeshesSize();
	glm::vec3 getAverageLocalPos();

	void calcAveragePos();

	int getBoneNum();
	std::vector<glm::mat4>::iterator getBoneOffsetBegin() { return boneInfo.offsetMatrix.begin(); }
	std::vector<glm::mat4>::iterator getBoneOffsetEnd() { return boneInfo.offsetMatrix.end(); }
	glm::mat4 getBoneOffset(int index);

	bool containBone(std::string nodeName);
	int getBoneToMap(std::string boneName);
	void setBoneInfo(int id, const glm::mat4 mat);

	void setGlobalInverseTransform(glm::mat4 mat) { globalInverseTransform = mat; }
	glm::mat4 getGlobalInverseTransform() { return globalInverseTransform; }

	int animationNum() { return animations.size(); }
	void setPose(std::string name, std::shared_ptr<Pose> pose);
	void setAnimation(std::shared_ptr<Animation> animation);
	void setAnimation(std::string name, std::shared_ptr<Animation> animation);
	void setAnimLoop(bool loop);
	std::array<glm::mat4, 250> getAnimationMatrix();
	std::array<glm::mat4, 250> getAnimationMatrix(float animationTime,std::string animationName);

	void setImageDataCount(uint32_t count) { imageDataCount = count; }
	uint32_t getImageDataCount() { return imageDataCount; }

	//�A�j���[�V������K�p�����邽�߂̍s����擾����
	glm::mat4* getAnimationMatrix(glm::mat4 mat);
};