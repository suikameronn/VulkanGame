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
	std::vector<std::shared_ptr<Meshes>> meshes;

	float startFrame;
	float endFrame;

	bool loopAnim;
	bool finish;

	glm::vec3 averageLocalPos;

	std::unordered_map<std::string, int> m_BoneNameToIndexMap;
	std::vector<glm::mat4> boneOffsets;

	std::unordered_map<std::string,std::shared_ptr<Animation>> animations;

	uint32_t imageDataCount;

public:

	FbxModel();

	void addMeshes(Meshes* mesh);

	int getBoneNum();
	glm::mat4 getBoneOffset(int index);

	std::shared_ptr<Meshes> getMeshes(uint32_t i);

	void cleanupVulkan();

	uint32_t getMeshesSize();
	glm::vec3 getAverageLocalPos();

	void calcAveragePos();

	bool containBone(std::string nodeName);
	int getBoneToMap(std::string boneName);
	void setBoneInfo(int id,const glm::mat4 mat);

	void setAnimation(std::string name, std::shared_ptr<Animation> animation);

	void setAnimLoop(bool loop);

	void setImageDataCount(uint32_t count) { imageDataCount = count; }
	uint32_t getImageDataCount() { return imageDataCount; }

	//アニメーションを適用させるための行列を取得する
	glm::mat4* getAnimationMatrix(glm::mat4 mat);
};