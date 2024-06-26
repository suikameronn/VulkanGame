#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include"Meshes.h"
#include"Material.h"

class FbxModel
{
private:
	std::vector<std::shared_ptr<Meshes>> meshes;

	float startFrame;
	float endFrame;

	bool loopAnim;
	bool finish;

	glm::vec3 averageLocalPos;

public:

	FbxModel();

	void addMeshes(Meshes* mesh);

	std::shared_ptr<Meshes> getMeshes(uint32_t i);

	void cleanupVulkan();

	uint32_t getMeshesSize();
	glm::vec3 getAverageLocalPos();

	void calcAveragePos();

	void setAnimLoop(bool loop);

	//アニメーションを適用させるための行列を取得する
	glm::mat4* getAnimationMatrix(glm::mat4 mat);
};