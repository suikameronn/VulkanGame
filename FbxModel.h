#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

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
	std::vector<BoneInfo> m_BoneInfo;
	std::vector<BoneData> m_Bones;

	std::unordered_map<std::string,std::shared_ptr<Animation>> animations;

public:

	FbxModel();

	void addMeshes(Meshes* mesh);

	void ReserveBones(int vertices) { m_Bones.resize(vertices); }
	int getBoneNum();

	std::shared_ptr<Meshes> getMeshes(uint32_t i);

	void cleanupVulkan();

	uint32_t getMeshesSize();
	glm::vec3 getAverageLocalPos();

	void calcAveragePos();

	int setBoneToMap(std::string boneName);
	void setBoneInfo(int id,const glm::mat4 mat);
	void addBoneData(int vertID, int boneID, float weight);

	void setAnimation(std::string name, std::shared_ptr<Animation> animatio);

	void setAnimLoop(bool loop);

	//アニメーションを適用させるための行列を取得する
	glm::mat4* getAnimationMatrix(glm::mat4 mat);
};